/**************************************************************************/
/*  gd_mono.h                                                             */
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

#ifndef GD_MONO_H
#define GD_MONO_H

#include "../godotsharp_defs.h"

#include "core/io/config_file.h"

#ifndef GD_CLR_STDCALL
#ifdef WIN32
#define GD_CLR_STDCALL __stdcall
#else
#define GD_CLR_STDCALL
#endif
#endif

namespace gdmono {

#if defined(TOOLS_ENABLED)
struct PluginCallbacks {
#ifdef LIBRARY_ENABLED
	using FuncLoadScriptAssembliesCallback = bool(GD_CLR_STDCALL *)();
	using FuncGetEditorPluginCallback = Object *(GD_CLR_STDCALL *)(const void **, int32_t);
	using FuncUnloadScriptAssembliesPluginCallback = bool(GD_CLR_STDCALL *)();
	FuncLoadScriptAssembliesCallback LoadScriptAssembliesCallback = nullptr;
	FuncGetEditorPluginCallback GetEditorPluginCallback = nullptr;
	FuncUnloadScriptAssembliesPluginCallback UnloadScriptAssembliesPluginCallback = nullptr;
#else
	using FuncLoadProjectAssemblyCallback = bool(GD_CLR_STDCALL *)(const char16_t *, String *);
	using FuncLoadToolsAssemblyCallback = Object *(GD_CLR_STDCALL *)(const char16_t *, const void **, int32_t);
	using FuncUnloadProjectPluginCallback = bool(GD_CLR_STDCALL *)();
	FuncLoadProjectAssemblyCallback LoadProjectAssemblyCallback = nullptr;
	FuncLoadToolsAssemblyCallback LoadToolsAssemblyCallback = nullptr;
	FuncUnloadProjectPluginCallback UnloadProjectPluginCallback = nullptr;
#endif
};
#endif
} // namespace gdmono
namespace GDMonoCache {
struct ManagedCallbacks;
}

#if defined(TOOLS_ENABLED)
#ifdef LIBRARY_ENABLED
using godot_plugins_initialize_fn = bool (*)(void *, gdmono::PluginCallbacks *, GDMonoCache::ManagedCallbacks *, const void **, int32_t);
#else
using godot_plugins_initialize_fn = bool (*)(void *, bool, gdmono::PluginCallbacks *, GDMonoCache::ManagedCallbacks *, const void **, int32_t);
#endif
#else
using godot_plugins_initialize_fn = bool (*)(void *, GDMonoCache::ManagedCallbacks *, const void **, int32_t);
#endif

class GDMono {
	bool initialized = false;
	bool runtime_initialized = false;
	bool finalizing_scripts_domain = false;

#ifndef LIBRARY_ENABLED
	void *hostfxr_dll_handle = nullptr;
	bool is_native_aot = false;
	String project_assembly_path;
	uint64_t project_assembly_modified_time = 0;
#endif
#ifdef DEBUG_METHODS_ENABLED
	uint64_t api_core_hash = 0;
#endif
#if defined(GD_MONO_HOT_RELOAD)
	int project_load_failure_count = 0;
#endif
#if defined(TOOLS_ENABLED) && !defined(LIBRARY_ENABLED)
	bool _load_project_assembly();
#endif

#ifdef TOOLS_ENABLED
	uint64_t api_editor_hash = 0;
#endif
	void _init_godot_api_hashes();

#if defined(TOOLS_ENABLED)
	gdmono::PluginCallbacks plugin_callbacks;
#endif

protected:
	static GDMono *singleton;

public:
#ifdef DEBUG_METHODS_ENABLED
	uint64_t get_api_core_hash() {
		if (api_core_hash == 0) {
			api_core_hash = ClassDB::get_api_hash(ClassDB::API_CORE);
		}
		return api_core_hash;
	}
#ifdef TOOLS_ENABLED
	uint64_t get_api_editor_hash() {
		if (api_editor_hash == 0) {
			api_editor_hash = ClassDB::get_api_hash(ClassDB::API_EDITOR);
		}
		return api_editor_hash;
	}
#endif // TOOLS_ENABLED
#endif // DEBUG_METHODS_ENABLED

	_FORCE_INLINE_ static String get_expected_api_build_config() {
#ifdef TOOLS_ENABLED
		return "Debug";
#else
#ifdef DEBUG_ENABLED
		return "Debug";
#else
		return "Release";
#endif
#endif
	}

	static GDMono *get_singleton() {
		return singleton;
	}

	_FORCE_INLINE_ bool is_initialized() const {
		return initialized;
	}
	_FORCE_INLINE_ bool is_runtime_initialized() const {
		return runtime_initialized;
	}
	_FORCE_INLINE_ bool is_finalizing_scripts_domain() {
		return finalizing_scripts_domain;
	}

#ifndef LIBRARY_ENABLED
	_FORCE_INLINE_ const String &get_project_assembly_path() const {
		return project_assembly_path;
	}
	_FORCE_INLINE_ uint64_t get_project_assembly_modified_time() const {
		return project_assembly_modified_time;
	}
#endif

#if defined(TOOLS_ENABLED)
	const gdmono::PluginCallbacks &get_plugin_callbacks() {
		return plugin_callbacks;
	}
#endif

#if defined(GD_MONO_HOT_RELOAD)
	void reload_failure();
	Error reload_project_assemblies();
#endif
#ifndef LIBRARY_ENABLED
	void initialize();
#else
	void initialize(godot_plugins_initialize_fn godot_plugins_initialize);
#endif
#if defined(TOOLS_ENABLED)
	void initialize_load_assemblies();
#endif

	GDMono();
	~GDMono();
};

namespace mono_bind {

class GodotSharp : public Object {
	GDCLASS(GodotSharp, Object);

	friend class GDMono;

	void _reload_assemblies(bool p_soft_reload);
	bool _is_runtime_initialized();

protected:
	static GodotSharp *singleton;
	static void _bind_methods();

public:
	static GodotSharp *get_singleton() { return singleton; }

	GodotSharp();
	~GodotSharp();
};

} // namespace mono_bind

#endif // GD_MONO_H
