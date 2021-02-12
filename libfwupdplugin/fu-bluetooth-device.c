/*
 * Copyright (C) 2021 Ricardo Cañuelo <ricardo.canuelo@collabora.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define G_LOG_DOMAIN				"FuBluetoothDevice"

#include "config.h"

#include "fu-device-private.h"
#include "fu-bluetooth-device.h"


/**
 * SECTION:fu-bluetooth-device
 * @short_description: a Bluetooth device
 *
 * An object that represents a Bluetooth device.
 *
 * See also: #FuDevice
 */

typedef struct {
	gchar		*name;
	gchar		*address;
	gchar		*adapter;
	GPtrArray	*uuids;		/* (element-type gchar *) */
	gboolean	 connected;
} FuBluetoothDevicePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (FuBluetoothDevice, fu_bluetooth_device, FU_TYPE_DEVICE)

enum {
	PROP_0,
	PROP_CONNECTED,
	PROP_LAST
};

#define GET_PRIVATE(o) (fu_bluetooth_device_get_instance_private (o))

/**
 * fu_bluetooth_device_get_name:
 * @dev: A #FuBluetoothDevice
 *
 * Gets the name of the device.
 *
 * Returns: The name of the device.
 *
 * Since: 1.5.5
 **/
const gchar *
fu_bluetooth_device_get_name (FuBluetoothDevice *dev)
{
	FuBluetoothDevicePrivate *priv = GET_PRIVATE (dev);

	return priv->name;
}

/**
 * fu_bluetooth_device_get_address:
 * @dev: A #FuBluetoothDevice
 *
 * Gets the address of the device.
 *
 * Returns: The address of the device.
 *
 * Since: 1.5.5
 **/
const gchar *
fu_bluetooth_device_get_address (FuBluetoothDevice *dev)
{
	FuBluetoothDevicePrivate *priv = GET_PRIVATE (dev);

	return priv->address;
}

static void
fu_bluetooth_device_get_property (GObject *object, guint prop_id,
				  GValue *value, GParamSpec *pspec)
{
	FuBluetoothDevice *self = FU_BLUETOOTH_DEVICE (object);
	FuBluetoothDevicePrivate *priv = GET_PRIVATE (self);

	switch (prop_id) {
	case PROP_CONNECTED:
		g_value_set_boolean (value, priv->connected);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
fu_bluetooth_device_set_property (GObject *object, guint prop_id,
				  const GValue *value, GParamSpec *pspec)
{
	FuBluetoothDevice *self = FU_BLUETOOTH_DEVICE (object);
	FuBluetoothDevicePrivate *priv = GET_PRIVATE (self);

	switch (prop_id) {
	case PROP_CONNECTED:
		priv->connected = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
fu_bluetooth_device_finalize (GObject *object)
{
	FuBluetoothDevice *self = FU_BLUETOOTH_DEVICE (object);
	FuBluetoothDevicePrivate *priv = GET_PRIVATE (self);

	g_free (priv->name);
	g_free (priv->address);
	g_free (priv->adapter);
	g_ptr_array_free (priv->uuids, TRUE);
	G_OBJECT_CLASS (fu_bluetooth_device_parent_class)->finalize (object);
}

static void
fu_bluetooth_device_init (FuBluetoothDevice *self)
{
}

static void
fu_bluetooth_device_class_init (FuBluetoothDeviceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	object_class->finalize = fu_bluetooth_device_finalize;
	object_class->get_property = fu_bluetooth_device_get_property;
	object_class->set_property = fu_bluetooth_device_set_property;

	pspec = g_param_spec_boolean ("connected", NULL, NULL, FALSE,
				     G_PARAM_READWRITE | G_PARAM_STATIC_NAME);
	g_object_class_install_property (object_class, PROP_CONNECTED, pspec);
}

/**
 * fu_bluetooth_device_new:
 * @name: The name of the bluetooth device.
 * @addr: The 48-bit address of the device.
 * @uuids: (element-type utf8): The UUIDs implemented by the device.
 * @connected: TRUE if the device is currently connected, FALSE otherwise.
 *
 * Creates a new #FuBluetoothDevice.
 *
 * Returns: (transfer full): a #FuBluetoothDevice
 *
 * Since: 1.5.5
 **/
FuBluetoothDevice *
fu_bluetooth_device_new (gchar *name, gchar *addr,
			 GPtrArray *uuids, gboolean connected)
{
	FuBluetoothDevice *self = g_object_new (FU_TYPE_BLUETOOTH_DEVICE, NULL);
	FuBluetoothDevicePrivate *priv = GET_PRIVATE (self);

	priv->name = name;
	priv->address = addr;
	priv->uuids = uuids;
	priv->connected = connected;
	return FU_BLUETOOTH_DEVICE (self);
}


