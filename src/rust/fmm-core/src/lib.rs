#[repr(C)]
pub struct FmmContext {
    _private: [u8; 0],
}

#[repr(C)]
pub struct FmmStringArray {
    pub data: *const *const std::os::raw::c_char,
    pub length: usize,
}

#[repr(C)]
pub struct FmmGameFsPlan {
    pub profile_id: *const std::os::raw::c_char,
    pub target_game_directory: *const std::os::raw::c_char,
    pub deployed_files: FmmStringArray,
}

#[no_mangle]
pub extern "C" fn fmm_context_create() -> *mut FmmContext {
    let ctx = Box::new(());
    Box::into_raw(ctx) as *mut FmmContext
}

#[no_mangle]
#[allow(clippy::missing_safety_doc)]
pub unsafe extern "C" fn fmm_context_destroy(ctx: *mut FmmContext) {
    if !ctx.is_null() {
        unsafe {
            let _ = Box::from_raw(ctx as *mut ());
        }
    }
}

#[no_mangle]
#[allow(clippy::missing_safety_doc)]
pub unsafe extern "C" fn fmm_gamefs_apply_plan(
    ctx: *mut FmmContext,
    plan: *const FmmGameFsPlan,
) -> i32 {
    if ctx.is_null() || plan.is_null() {
        return -1;
    }

    unsafe {
        let plan_ref = &*plan;
        if plan_ref.profile_id.is_null() || plan_ref.target_game_directory.is_null() {
            return -1;
        }

        let _profile_id = std::ffi::CStr::from_ptr(plan_ref.profile_id)
            .to_str()
            .unwrap_or("");
        let _target_game_directory = std::ffi::CStr::from_ptr(plan_ref.target_game_directory)
            .to_str()
            .unwrap_or("");

        if plan_ref.deployed_files.length > 0 {
            if plan_ref.deployed_files.data.is_null() {
                return -1;
            }
            let slice = std::slice::from_raw_parts(
                plan_ref.deployed_files.data,
                plan_ref.deployed_files.length,
            );
            for &ptr in slice {
                if ptr.is_null() {
                    return -1;
                }
                let _ = std::ffi::CStr::from_ptr(ptr).to_str().unwrap_or("");
            }
        }
    }

    0
}
