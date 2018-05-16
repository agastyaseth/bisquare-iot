// Copyright 2015 The Weave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "uweave/gatt.h"

/**
 * Weave base UUID for BLE UUID's in big endian format.
 * 00000000-0004-1000-8000-001A11000100
 */
const UwBleUuid UwBaseUuid = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x10, 0x00,  // UUID
     0x80, 0x00, 0x00, 0x1A, 0x11, 0x00, 0x01, 0x00},
    0x0000  // Short UUID
};

/**
 * Weave service UUID in big endian format.
 * 00000100-0004-1000-8000-001A11000100
 */
const UwBleUuid UwServiceUuid = {
    {0x00, 0x00, 0x01, 0x00, 0x00, 0x04, 0x10, 0x00,  // UUID
     0x80, 0x00, 0x00, 0x1A, 0x11, 0x00, 0x01, 0x00},
    0x0100  // Short UUID
};

/**
 * RX Characteristic UUID in big endian format.
 * 00000102-0004-1000-8000-001a11000100
 */
const UwBleUuid UwClientRxCharacteristicUuid = {
    {0x00, 0x00, 0x01, 0x02, 0x00, 0x04, 0x10, 0x00,  // UUID
     0x80, 0x00, 0x00, 0x1A, 0x11, 0x00, 0x01, 0x00},
    0x0102  // Short UUID
};

/**
 * TX Characteristic UUID in big endian format.
 * 00000101-0004-1000-8000-001a11000100
 */
const UwBleUuid UwClientTxCharacteristicUuid = {
    {0x00, 0x00, 0x01, 0x01, 0x00, 0x04, 0x10, 0x00,  // UUID
     0x80, 0x00, 0x00, 0x1A, 0x11, 0x00, 0x01, 0x00},
    0x0101  // Short UUID
};