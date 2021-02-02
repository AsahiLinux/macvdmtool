//
// Copyright © 2019 osy86. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef AppleHPMLib_h
#define AppleHPMLib_h

#include <IOKit/IOCFPlugIn.h>

#define kAppleHPMLibType CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault, \
    0x12, 0xA1, 0xDC, 0xCF, 0xCF, 0x7A, 0x47, 0x75, \
    0xBE, 0xE5, 0x9C, 0x43, 0x19, 0xF4, 0xCD, 0x2B  \
)
#define kAppleHPMLibInterface CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault, \
    0xC1, 0x3A, 0xCD, 0xD9, 0x20, 0x9E, 0x4B, 0x01, \
    0xB7, 0xBE, 0xE0, 0x5C, 0xD8, 0x83, 0xC7, 0xB1  \
)

typedef struct {
    IUNKNOWN_C_GUTS;
    uint16_t field_20;
    uint16_t field_22;
    IOReturn (*Read)(void *, uint64_t chipAddr, uint8_t dataAddr, void *buffer, uint64_t maxLen, uint32_t flags, uint64_t *readLen);
    IOReturn (*Write)(void *, uint64_t chipAddr, uint8_t dataAd6dr, const void *buffer, uint64_t len, uint32_t flags);
    IOReturn (*Command)(void *, uint64_t chipAddr, uint32_t cmd, uint32_t flags);
    IOReturn (*field_40)(void);
    IOReturn (*field_48)(void);
    IOReturn (*field_50)(void);
} AppleHPMLib;

#endif /* AppleHPMLib_h */
