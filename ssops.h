/* SPDX-License-Identifier: Apache-2.0 */
/* Based on https://github.com/osy/ThunderboltPatcher */

#include <cstddef>
#include <cstdlib>
#include <sstream>

template <typename T> std::stringstream &put(std::stringstream &str, const T &value)
{
    union coercion {
        T value;
        char data[sizeof(T)];
    };

    coercion c;

    c.value = value;

    str.write(c.data, sizeof(T));

    return str;
}

template <typename T> std::stringstream &get(std::stringstream &str, T &value)
{
    union coercion {
        T value;
        char data[sizeof(T)];
    };

    coercion c;

    c.value = value;

    str.read(c.data, sizeof(T));

    value = c.value;

    return str;
}
