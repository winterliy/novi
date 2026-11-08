#[no_mangle]
pub unsafe extern "C" fn novi_ascii_prefix(data: *const u8, length: usize) -> usize {
    let mut i = 0;
    while i < length && *data.add(i) < 0x80 { i += 1; }
    i
}
