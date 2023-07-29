/**************************************************************************/
/*  libgodot.cpp                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "core/extension/gdextension_interface.h"
#ifdef LIBRARY_ENABLED
#include "core/extension/gdextension_manager.h"
#include "core/variant/callable.h"
#include "libgodot.h"
#include "libgodot_callable.h"
#ifdef MODULE_MONO_ENABLED
#include "modules/mono/csharp_script.h"
#endif

uint32_t (*lib_godot_callable_hash)(void *);
void *(*lib_godot_get_as_text)(void *);
uint64_t (*lib_godot_get_object)(void *);
void (*lib_godot_disposes)(void *);
void (*lib_godot_call)(void *, const void *, int, void *, void *);

#ifdef __cplusplus
extern "C" {
#endif

GDExtensionBool (*initialization_function)(GDExtensionInterfaceGetProcAddress, GDExtensionClassLibraryPtr, GDExtensionInitialization *);
void (*scene_load_function)(Variant *);
void (*project_settings_load_function)(void *settings);
#ifdef MODULE_MONO_ENABLED
void (*project_settings_load_mono_function)();
GDMono *gdmono_instance;
#endif

LIBGODOT_API void libgodot_bind_custom_callable(uint32_t (*callable_hash_bind)(void *), void *(*get_as_text_bind)(void *), uint64_t (*get_object_bind)(void *), void (*disposes_bind)(void *), void (*call_bind)(void *, const void *, int, void *, void *)) {
	lib_godot_callable_hash = callable_hash_bind;
	lib_godot_get_as_text = get_as_text_bind;
	lib_godot_get_object = get_object_bind;
	lib_godot_disposes = disposes_bind;
	lib_godot_call = call_bind;
}

#ifdef MODULE_MONO_ENABLED
LIBGODOT_API void libgodot_bind_mono(godot_plugins_initialize_fn godot_plugins_initialize, void (*project_settings_load_mono_function_bind)()) {
	gdmono_instance = memnew(GDMono);
	gdmono_instance->initialize(godot_plugins_initialize);
	project_settings_load_mono_function = project_settings_load_mono_function_bind;
}
#endif
LIBGODOT_API void libgodot_bind(GDExtensionBool (*initialization_bind)(GDExtensionInterfaceGetProcAddress, GDExtensionClassLibraryPtr, GDExtensionInitialization *), void (*scene_function_bind)(Variant *), void (*project_settings_load_function_bind)(void *)) {
	initialization_function = initialization_bind;
	scene_load_function = scene_function_bind;
	project_settings_load_function = project_settings_load_function_bind;
}

LIBGODOT_API void *libgodot_create_callable(void *targetObject) {
	return new Callable(new LibGodotCallable(targetObject));
}

LIBGODOT_API bool is_editor_build() {
#ifdef TOOLS_ENABLED
	return true;
#else
	return false;
#endif
}

#ifdef MODULE_MONO_ENABLED
void libgodot_init_mono() {
	CSharpLanguage::get_singleton()->setup_mono(gdmono_instance);
}
#endif

void libgodot_project_settings_load(void *settings) {
	if (project_settings_load_function != nullptr) {
		project_settings_load_function(settings);
	}
#ifdef MODULE_MONO_ENABLED
	if (project_settings_load_mono_function != nullptr) {
		project_settings_load_mono_function();
	}
#endif
}

void libgodot_scene_load(Variant *scene) {
	if (scene_load_function != nullptr) {
		scene_load_function(scene);
	}
}

bool libgodot_is_scene_loadable() {
	return scene_load_function != nullptr;
}

void libgodot_init_resource() {
	if (initialization_function != nullptr) {
		Ref<GDExtension> libgodot;
		libgodot.instantiate();
		Error err = libgodot->initialize_extension_function(initialization_function, "LibGodot");
		if (err != OK) {
			ERR_PRINT("LibGodot Had an error initialize_extension_function'");
		} else {
			print_verbose("LibGodot initialization");
			libgodot->set_path("res://LibGodotGDExtension");
			GDExtensionManager::get_singleton()->load_extension("res://LibGodotGDExtension");
		}
	}
}

#ifdef __cplusplus
}
#endif

bool LibGodotCallable::_equal_func(const CallableCustom *p_a, const CallableCustom *p_b) {
	const LibGodotCallable *a = static_cast<const LibGodotCallable *>(p_a);
	const LibGodotCallable *b = static_cast<const LibGodotCallable *>(p_b);

	return (a->customObject == b->customObject);
}

bool LibGodotCallable::_less_func(const CallableCustom *p_a, const CallableCustom *p_b) {
	const LibGodotCallable *a = static_cast<const LibGodotCallable *>(p_a);
	const LibGodotCallable *b = static_cast<const LibGodotCallable *>(p_b);

	return (a->customObject == b->customObject);
}

uint32_t LibGodotCallable::hash() const {
	return lib_godot_callable_hash(customObject);
}

String LibGodotCallable::get_as_text() const {
	return *(String *)lib_godot_get_as_text(customObject);
}

ObjectID LibGodotCallable::get_object() const {
	return (ObjectID)lib_godot_get_object(customObject);
}

LibGodotCallable::LibGodotCallable(void *target) {
	customObject = target;
}

LibGodotCallable::~LibGodotCallable() {
	lib_godot_disposes(customObject);
}

CallableCustom::CompareEqualFunc LibGodotCallable::get_compare_equal_func() const {
	return _equal_func;
}

CallableCustom::CompareLessFunc LibGodotCallable::get_compare_less_func() const {
	return _less_func;
}

void LibGodotCallable::call(const Variant **p_arguments, int p_argcount, Variant &r_return_value, Callable::CallError &r_call_error) const {
	Callable::CallError callar = {};
	Variant return_value = {};
	lib_godot_call(customObject, static_cast<const void *>(p_arguments), p_argcount, (void *)&return_value, (void *)&callar);
	r_return_value = return_value;
	r_call_error = callar;
}

#endif
