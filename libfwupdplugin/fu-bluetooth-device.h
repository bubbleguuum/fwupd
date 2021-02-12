/*
 * Copyright (C) 2021 Ricardo Cañuelo <ricardo.canuelo@collabora.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <glib-object.h>
#include "fu-plugin.h"

#define FU_TYPE_BLUETOOTH_DEVICE (fu_bluetooth_device_get_type ())
G_DECLARE_DERIVABLE_TYPE (FuBluetoothDevice, fu_bluetooth_device, FU, BLUETOOTH_DEVICE, FuDevice)

struct _FuBluetoothDeviceClass
{
	FuDeviceClass	parent_class;
	gboolean	 (*probe)			(FuBluetoothDevice	*device,
							 GError			**error);
	gboolean	 (*open)			(FuBluetoothDevice	*device,
							 GError			**error);
	gboolean	 (*close)			(FuBluetoothDevice	*device,
							 GError			**error);
	void		 (*to_string)			(FuBluetoothDevice	*self,
							 guint			 indent,
							 GString		*str);
	gpointer	__reserved[28];
};


FuBluetoothDevice	*fu_bluetooth_device_new		(gchar *name, gchar *addr,
								 GPtrArray *uuids, gboolean connected);
const gchar		*fu_bluetooth_device_get_name 		(FuBluetoothDevice *dev);
const gchar		*fu_bluetooth_device_get_address 	(FuBluetoothDevice *dev);
