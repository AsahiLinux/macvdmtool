/* SPDX-License-Identifier: Apache-2.0 */
/* Based on https://github.com/osy/ThunderboltPatcher */

#include "AppleHPMLib.h"
#include "ssops.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFString.h>

struct failure : public std::runtime_error {
    failure(const char *x) : std::runtime_error(x)
    {
    }
};

struct IOObjectDeleter {
    io_object_t arg;

    IOObjectDeleter(io_object_t arg) : arg(arg)
    {
    }

    ~IOObjectDeleter()
    {
        IOObjectRelease(arg);
    }
};

struct HPMPluginInstance {
    IOCFPlugInInterface **plugin = nullptr;
    AppleHPMLib **device;

    HPMPluginInstance(io_service_t service)
    {
        SInt32 score;
        IOReturn ret = IOCreatePlugInInterfaceForService(service, kAppleHPMLibType,
                                                         kIOCFPlugInInterfaceID, &plugin, &score);
        if (ret != kIOReturnSuccess)
            throw failure("IOCreatePlugInInterfaceForService failed");

        HRESULT res = (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kAppleHPMLibInterface),
                                                (LPVOID *)&device);
        if (res != S_OK)
            throw failure("QueryInterface failed");
    }

    ~HPMPluginInstance()
    {
        if (plugin) {
            printf("Exiting DBMa mode... ");
            if (this->command(0, 'DBMa', "\x00") == 0)
                printf("OK\n");
            else
                printf("Failed\n");
            IODestroyPlugInInterface(plugin);
        }
    }

    std::string readRegister(uint64_t chipAddr, uint8_t dataAddr, int flags = 0)
    {
        std::string ret;
        ret.resize(64);
        uint64_t rlen = 0;
        IOReturn x = (*device)->Read(device, chipAddr, dataAddr, &ret[0], 64, flags, &rlen);
        if (x != 0)
            throw failure("readRegister failed");
        return ret;
    }

    void writeRegister(uint64_t chipAddr, uint8_t dataAddr, std::string value)
    {
        IOReturn x = (*device)->Write(device, chipAddr, dataAddr, &value[0], value.length(), 0);
        if (x != 0)
            throw failure("writeRegister failed");
    }

    int command(uint64_t chipAddr, uint32_t cmd, std::string args = "")
    {
        if (args.length())
            (*device)->Write(device, chipAddr, 9, args.data(), args.length(), 0);
        auto ret = (*device)->Command(device, chipAddr, cmd, 0);
        if (ret)
            return -1;
        auto res = this->readRegister(chipAddr, 9);
        return res[0] & 0xfu;
    }
};

uint32_t GetUnlockKey()
{
    CFMutableDictionaryRef matching = IOServiceMatching("IOPlatformExpertDevice");
    if (!matching)
        throw failure("IOServiceMatching failed (IOPED)");

    io_iterator_t iter = 0;
    IOObjectDeleter iterDel(iter);
    io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
    if (!service)
        throw failure("IOServiceGetMatchingService failed (IOPED)");

    IOObjectDeleter deviceDel(service);

    io_name_t deviceName;
    if (IORegistryEntryGetName(service, deviceName) != kIOReturnSuccess) {
        throw failure("IORegistryEntryGetName failed (IOPED)");
    }

    printf("Mac type: %s\n", deviceName);

    return (deviceName[0] << 24) | (deviceName[1] << 16) | (deviceName[2] << 8) | deviceName[3];
}

std::unique_ptr<HPMPluginInstance> FindDevice()
{
    std::unique_ptr<HPMPluginInstance> ret;

    printf("Looking for HPM devices...\n");

    CFMutableDictionaryRef matching = IOServiceMatching("AppleHPM");
    if (!matching)
        throw failure("IOServiceMatching failed");

    io_iterator_t iter = 0;
    IOObjectDeleter iterDel(iter);
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iter) != kIOReturnSuccess)
        throw failure("IOServiceGetMatchingServices failed");

    io_service_t device;
    while ((device = IOIteratorNext(iter))) {
        IOObjectDeleter deviceDel(device);
        io_string_t pathName;

        if (IORegistryEntryGetPath(device, kIOServicePlane, pathName) != kIOReturnSuccess) {
            fprintf(stderr, "Failed to get device path for object %d.", device);
            continue;
        }

        CFNumberRef data;
        int32_t rid;
        data = (CFNumberRef)IORegistryEntryCreateCFProperty(device, CFSTR("RID"),
                                                            kCFAllocatorDefault, 0);
        if (!data)
            throw failure("No RID");

        CFNumberGetValue(data, kCFNumberSInt32Type, &rid);
        CFRelease(data);

        // RID=0 seems to always be the right port
        if (rid != 0)
            continue;

        printf("Found: %s\n", pathName);
        ret = std::make_unique<HPMPluginInstance>(device);
    }

    if (!ret)
        throw failure("No matching devices");

    return ret;
};

void UnlockAce(HPMPluginInstance &inst, int no, uint32_t key)
{
    printf("Unlocking... ");
    std::stringstream args;
    put(args, key);
    if (inst.command(no, 'LOCK', args.str())) {
        printf(" Failed.\n");
        printf("Trying to reset... ");
        if (inst.command(no, 'Gaid')) {
            printf("Failed.\n");
            throw failure("Failed to unlock device");
        }
        printf("OK.\nUnlocking... ");
        if (inst.command(no, 'LOCK', args.str())) {
            printf(" Failed.\n");
            throw failure("Failed to unlock device");
        }
    }

    printf("OK\n");
}

void DoVDM(HPMPluginInstance &inst, int no, std::vector<uint32_t> vdm)
{

    auto rs = inst.readRegister(no, 0x4d);
    uint8_t rxst = rs[0];

    std::stringstream args;
    put(args, (uint8_t)(((3 << 4) | vdm.size())));
    for (uint32_t i : vdm)
        put(args, i);

    auto v = args.str();

    if (inst.command(no, 'VDMs', args.str()))
        throw failure("Failed to send VDM\n");

    int i;
    for (i = 0; i < 16; i++) {
        rs = inst.readRegister(no, 0x4d);
        if ((uint8_t)rs[0] != rxst)
            break;
    }
    if (i >= 16)
        throw failure("Did not get a reply to VDM\n");

    uint32_t vdmhdr;
    std::stringstream reply;
    reply.str(rs);
    get(reply, rxst);
    get(reply, vdmhdr);

    if (vdmhdr != (vdm[0] | 0x40)) {
        printf("VDM failed (reply: 0x%08x)\n", vdmhdr);
        throw failure("VDM failed");
    }
}

int DoSerial(HPMPluginInstance &inst, int no)
{
    printf("Putting target into serial mode... ");

    std::vector<uint32_t> serial{0x5ac8012, 0x1840306};
    DoVDM(inst, no, serial);

    printf("OK\n");

    printf("Putting local end into serial mode... ");

    std::stringstream args;
    put(args, (uint32_t)0x1840306);

    if (inst.command(no, 'DVEn', args.str())) {
        printf("Failed.\n");
        return 1;
    }
    printf("OK\n");

    return 0;
}

int DoReboot(HPMPluginInstance &inst, int no)
{
    printf("Rebooting target into normal mode... ");

    std::vector<uint32_t> serial{0x5ac8012, 0x105, 0x80000000};
    DoVDM(inst, no, serial);

    printf("OK\n");
    return 0;
}

int DoRebootSerial(HPMPluginInstance &inst, int no)
{

    if (DoReboot(inst, no))
        return 1;

    printf("Waiting for connection...");
    fflush(stdout);

    sleep(1);
    int i;
    for (i = 0; i < 30; i++) {
        printf(".");
        fflush(stdout);
        auto t = inst.readRegister(no, 0x3f);
        if (t[0] & 1)
            break;
        usleep(100000);
    }
    if (i >= 30) {
        printf(" Timed out\n");
        return 1;
    }
    printf(" Connected\n");
    sleep(1);

    return DoSerial(inst, no);
}

int DoDFU(HPMPluginInstance &inst, int no)
{
    printf("Rebooting target into DFU mode... ");

    std::vector<uint32_t> serial{0x5ac8012, 0x106, 0x80010000};
    DoVDM(inst, no, serial);

    printf("OK\n");
    return 0;
}

int main2(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <command>\n", argv[0]);
        printf("Commands:\n");
        printf("  serial - enter serial mode on both ends\n");
        printf("  reboot - reboot the target\n");
        printf("  reboot serial - reboot the target and enter serial mode\n");
        printf("  dfu - put the target into DFU mode\n");
        printf("  nop - do nothing\n");
        return 1;
    }

    uint32_t key = GetUnlockKey();

    auto inst = FindDevice();
    int no = 0;

    auto t = inst->readRegister(no, 0x3f);
    std::string type = (t[0] & 1) ? ((t[0] & 2) == 0 ? "Source" : "Sink") : "None";
    printf("Connection: %s\n", type.c_str());

    if (!(t[0] & 1))
        throw failure("No connection detected");

    auto res = inst->readRegister(no, 0x03);
    res.erase(res.find('\0'));
    printf("Status: %s\n", res.c_str());

    if (res != "DBMa") {
        UnlockAce(*inst, no, key);
        printf("Entering DBMa mode... ");

        if (inst->command(no, 'DBMa', "\x01"))
            throw failure("Failed to enter DBMa mode");

        res = inst->readRegister(no, 0x03);
        res.erase(res.find('\0'));

        printf("Status: %s\n", res.c_str());
        if (res != "DBMa")
            throw failure("Failed to enter DBMa mode");
    }

    std::string cmd = argv[1];
    std::string arg = "";

    if (argc >= 3)
        arg = argv[2];

    if (cmd == "serial")
        return DoSerial(*inst, no);
    else if (cmd == "reboot") {
        if (arg == "serial")
            return DoRebootSerial(*inst, no);
        else
            return DoReboot(*inst, no);
    } else if (cmd == "dfu")
        return DoDFU(*inst, no);
    else if (cmd == "nop")
        return 0;

    printf("Unknown command\n");
    return 1;
}

int main(int argc, char **argv)
{
    // This makes sure we call the HPMPluginInstance destructor.
    try {
        return main2(argc, argv);
    } catch (failure e) {
        printf("%s\n", e.what());
        return -1;
    }
}
