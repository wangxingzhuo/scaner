#[link(name = "avformat")]
#[link(name = "avutil")]
extern "C" {
    // pub fn puts(filename: *const std::os::raw::c_char);
    pub fn put_meta(filename: *const std::os::raw::c_char) -> std::os::raw::c_int;
}
