/*
 * Copyright (C) 2021 Ricardo Cañuelo <ricardo.canuelo@collabora.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define G_LOG_DOMAIN				"FuBackend"

#include "config.h"

#include "fu-bluetooth-device.h"
#include "fu-bluez-backend.h"

struct _FuBluezBackend {
	FuBackend		 parent_instance;
	GDBusConnection		*connection;
	GHashTable		*devices;	/* platform_id : * FuDevice */
};

G_DEFINE_TYPE (FuBluezBackend, fu_bluez_backend, FU_TYPE_BACKEND)


/*
 * Returns a new FuBluetoothDevice populated with the properties passed
 * in the properties variant (@a{sv}).
 */
static FuBluetoothDevice *
fu_bluez_load_device_properties (GVariant *properties)
{
	const gchar *prop_name;
	GVariantIter it;
	g_autoptr(GVariant) prop_val = NULL;
	FuBluetoothDevice *dev = NULL;
	gchar *address;
	gchar *name;
	gboolean connected;
	GPtrArray *uuids;

	uuids = g_ptr_array_new_with_free_func (g_free);
	g_variant_iter_init (&it, properties);
	while (g_variant_iter_next (&it, "{&sv}", &prop_name, &prop_val)) {
		if (g_strcmp0 (prop_name, "Address") == 0) {
			address = g_strdup (g_variant_get_string(prop_val, NULL));
			continue;
		}
		if (g_strcmp0 (prop_name, "Name") == 0) {
			name = g_strdup (g_variant_get_string(prop_val, NULL));
			continue;
		}
		if (g_strcmp0 (prop_name, "Connected") == 0) {
			connected = g_variant_get_boolean(prop_val);
			continue;
		}
		if (g_strcmp0 (prop_name, "UUIDs") == 0) {
			GVariantIter uuids_it;
			const gchar *uuid_str;

			g_variant_iter_init (&uuids_it, prop_val);
			while (g_variant_iter_next (&uuids_it, "&s", &uuid_str)) {
				g_ptr_array_add (uuids, g_strdup (uuid_str));
			}
			continue;
		}
	}
	dev = fu_bluetooth_device_new (name, address, uuids, connected);

	return dev;
}


static gboolean
fu_bluez_backend_setup (FuBackend *backend, GError **error)
{
	FuBluezBackend *self = FU_BLUEZ_BACKEND (backend);

	self->connection = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, NULL);
	if (self->connection == NULL) {
		g_prefix_error (error, "Failed to connect to bluez dbus: ");
		return FALSE;
	}
	/*
	 * TODO: Subscribe DBus signals.
	 */

	return TRUE;
}

static gboolean
fu_bluez_backend_coldplug (FuBackend *backend, GError **error)
{
	FuBluezBackend *self = FU_BLUEZ_BACKEND (backend);
	g_autoptr(GDBusProxy) proxy = NULL;
	g_autoptr(GVariant) value = NULL;
	g_autoptr(GVariant) obj_info = NULL;
	const gchar *obj_path;
	GVariantIter i;

	/*
	 * Bluez publishes all the object info through the
	 * "GetManagedObjects" method
	 * ("org.freedesktop.DBus.ObjectManager" interface).
	 *
	 * Look for objects that implement "org.bluez.Device1".
	 */

	proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
					       G_DBUS_PROXY_FLAGS_NONE,
					       NULL,
					       "org.bluez",
					       "/",
					       "org.freedesktop.DBus.ObjectManager",
					       NULL,
					       error);
	if (proxy == NULL) {
		g_prefix_error (error, "Failed to connect to bluez dbus: ");
		return FALSE;
	}

	value = g_dbus_proxy_call_sync (proxy,
					"GetManagedObjects",
					NULL,
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					error);
	if (value == NULL) {
		g_prefix_error (error, "Failed to call GetManagedObjects: ");
		return FALSE;
	}

	value = g_variant_get_child_value (value, 0);
	g_variant_iter_init (&i, value);
	while (g_variant_iter_next (&i, "{&o@a{sa{sv}}}", &obj_path, &obj_info)) {
		const gchar *if_name;
		g_autoptr(GVariant) properties = NULL;
		GVariantIter j;

		g_variant_iter_init (&j, obj_info);
		while (g_variant_iter_next (&j, "{&s@a{sv}}", &if_name, &properties)) {
			g_autoptr(FuBluetoothDevice) dev = NULL;

			if (g_strcmp0 (if_name, "org.bluez.Device1")) {
				continue;
			}

			dev = fu_bluez_load_device_properties (properties);
			if (dev) {
				g_hash_table_insert (self->devices,
						     g_strdup (fu_bluetooth_device_get_address (dev)),
						     g_object_ref (dev));
				fu_backend_device_added (FU_BACKEND (self),
							 FU_DEVICE (dev));
			}
		}
	}

	return TRUE;
}

static void
fu_bluez_backend_finalize (GObject *object)
{
	FuBluezBackend *self = FU_BLUEZ_BACKEND (object);

	g_hash_table_unref (self->devices);
	g_object_unref (self->connection);
	G_OBJECT_CLASS (fu_bluez_backend_parent_class)->finalize (object);
}

static void
fu_bluez_backend_init (FuBluezBackend *self)
{
	self->devices = g_hash_table_new_full (g_str_hash, g_str_equal,
					       g_free,
					       (GDestroyNotify) g_object_unref);
}

static void
fu_bluez_backend_class_init (FuBluezBackendClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	FuBackendClass *klass_backend = FU_BACKEND_CLASS (klass);

	object_class->finalize = fu_bluez_backend_finalize;
	klass_backend->setup = fu_bluez_backend_setup;
	klass_backend->coldplug = fu_bluez_backend_coldplug;
	/*
	 * TODO: Needed?
	 *
	 * klass_backend->recoldplug = fu_bluez_backend_recoldplug;
	 */
}

FuBackend *
fu_bluez_backend_new (void)
{
	return FU_BACKEND (g_object_new (FU_TYPE_BLUEZ_BACKEND, "name", "bluez", NULL));
}
