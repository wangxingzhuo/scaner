use std::{os, ffi::CString};

#[link(name = "avformat")]
#[link(name = "avcodec")]
#[link(name = "avutil")]
extern "C" {
    fn load_audio_meta(filename: *const os::raw::c_char) -> os::raw::c_int;
    fn load_png_meta(filename: *const os::raw::c_char) -> os::raw::c_int;
    fn load_jpg_meta(filename: *const os::raw::c_char) -> os::raw::c_int;
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

pub fn load_audio(filename: &str) -> i32 {
    let ret: i32;

    unsafe {
        let c_file = CString::new(filename).expect("").into_raw();
        ret = load_audio_meta(c_file);
    }
    ret
}

pub fn load_image(filename: &str) -> i32 {
    let ret: i32;

    unsafe {
        let c_file = CString::new(filename).expect("").into_raw();
        ret = load_png_meta(c_file);
    }
    ret
}
