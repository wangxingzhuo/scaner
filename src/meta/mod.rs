use std::{os, ffi::CString};

#[link(name = "avformat")]
#[link(name = "avcodec")]
#[link(name = "avutil")]
extern "C" {
    fn load_audio_meta(filename: *const os::raw::c_char, buf: *const os::raw::c_uchar, size: os::raw::c_uint) -> os::raw::c_int;
    fn print_pic_meta(
        filename: *const os::raw::c_char
    ) -> os::raw::c_uint;
}

pub enum EnMediaType {
    UNKNOW,
    AUDIO,
    IMAGE
}

pub fn media_type(ext_name: &str) -> EnMediaType {
    match ext_name {
        // audio
        "wav" => EnMediaType::AUDIO,
        "flac" => EnMediaType::AUDIO,
        "aac" => EnMediaType::AUDIO,
        "ape" => EnMediaType::AUDIO,
        "aiff" => EnMediaType::AUDIO,
        "mp3" => EnMediaType::AUDIO,
        "wma" => EnMediaType::AUDIO,
        // picture
        "png" => EnMediaType::IMAGE,
        "jpg" => EnMediaType::IMAGE,
        "jpeg" => EnMediaType::IMAGE,
        "webp" => EnMediaType::IMAGE,
        &_ => EnMediaType::UNKNOW
    }
}

pub fn load_audio(buf: *const os::raw::c_uchar, filename: &str, size: os::raw::c_uint) -> i32 {
    let ret: i32;

    unsafe {
        let c_file = CString::new(filename).expect("").into_raw();
        ret = load_audio_meta(c_file, buf, size);
    }
    ret
}

pub fn load_image(filename: &str) {
    unsafe {
        let c_file = CString::new(filename).expect("").into_raw();
        print_pic_meta(c_file);
    }
}
