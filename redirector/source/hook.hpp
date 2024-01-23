#ifndef REDIRECTOR_HOOK_HPP_
#define REDIRECTOR_HOOK_HPP_

// A simple wrapper over MinHook.
class MinHook
{
public:
	// Wrapper over function hook.
	// @tparam Original - orignal function.
	// @tparam Detour - detour function.
	template <auto Original, auto Detour>
	struct FunctionHook
	{
		using FunctionPtr = decltype(Original);

		static inline FunctionPtr s_Target		= Original;
		static inline FunctionPtr s_Original	= nullptr;

		// Creates function hook in inactive state.
		static MH_STATUS Create() {
			return MinHook::CreateHook(s_Target, Detour, &s_Original);
		}

		// Enables function hook.
		static MH_STATUS Enable() {
			return MinHook::EnableHook(s_Target);
		}

		// Creates and enables function hook.
		static MH_STATUS CreateAndEnable() 
		{
			auto status = Create();
			if (status == MH_OK)
				status = Enable();

			return status;
		}

		// Deactivates the specified function hook.
		static MH_STATUS Disable() {
			return MinHook::DisableHook(s_Target);
		}

		// Removes function hook.
		static MH_STATUS Remove() {
			return MinHook::RemoveHook(s_Target);
		}
	};

	// MinHook initialization.
	static MH_STATUS Initialize() {
		return MH_Initialize();
	}

	// MinHook uninitialization.
	static MH_STATUS Uninitialize() {
		return MH_Uninitialize();
	}

	// Sets a hook on the specified function in the inactive state
	template <typename Function>
	static MH_STATUS CreateHook(Function* target, Function* hook, Function** original) {
		return MH_CreateHook(reinterpret_cast<LPVOID>(target), reinterpret_cast<LPVOID>(hook), reinterpret_cast<LPVOID*>(original));
	}

	// Activates the specified function hook.
	template <typename Function>
	static MH_STATUS EnableHook(Function function) {
		return MH_EnableHook(reinterpret_cast<LPVOID>(function));
	}

	// Removes the hook on the specified function
	template <typename Function>
	static MH_STATUS RemoveHook(Function* target) {
		return MH_RemoveHook(target);
	}

	// Deactivates the specified function hook.
	template <typename Function>
	static MH_STATUS DisableHook(Function function) {
		return MH_DisableHook(function);
	}

	// Activates all hooks.
	static MH_STATUS EnableAllHooks() {
		return MH_EnableHook(MH_ALL_HOOKS);
	}

	// Deactivates all hooks
	static MH_STATUS DisableAllHooks() {
		return MH_DisableHook(MH_ALL_HOOKS);
	}
};

#endif // !REDIRECTOR_HOOK_HPP_
