mod dir;
mod sound;

use std::{self, ffi::CString};
use String;

fn main() {
    let mut args = std::env::args();
    let root = if 1 < args.len() {
        args.nth(1).unwrap()
    } else {
        String::from(".")
    };
    println!("{}", root.as_str());
    let list = dir::walk_dir(std::path::Path::new(&root), 1);
    for row in list {
        let filename = row.to_str().unwrap();
        let ext = row.extension();
        if None == ext {
            continue;
        }
        let ext_name = ext.unwrap().to_str().unwrap();
        if "mp3".eq(ext_name) || "flac".eq(ext_name) || "wav".eq(ext_name) {
            println!("> {:?}", filename);
            unsafe {
                let c_file = CString::new(filename).expect("").into_raw();
                sound::put_meta(c_file);
            }
        }
    }
}
