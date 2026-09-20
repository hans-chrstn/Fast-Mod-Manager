#[repr(C)]
pub struct FmmContext {
    _private: [u8; 0],
}

#[no_mangle]
pub extern "C" fn fmm_context_create() -> *mut FmmContext {
    let ctx = Box::new(());
    Box::into_raw(ctx) as *mut FmmContext
}

#[no_mangle]
pub extern "C" fn fmm_context_destroy(ctx: *mut FmmContext) {
    if !ctx.is_null() {
        unsafe {
            let _ = Box::from_raw(ctx as *mut ());
        }
    }
}
