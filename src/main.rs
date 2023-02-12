mod dir;
mod meta;

use std::{env, path::Path, os::raw, alloc::alloc, alloc::Layout};
use meta::EnMediaType;

fn main() {
    let mut args = env::args();
    let root = if 1 < args.len() { args.nth(1).unwrap() } else { String::from("/home/james/图片/") };
    // println!("{}", root.as_str());
    let list = dir::walk(Path::new(&root));
    let layout = Layout::array::<i8>(8192).expect("overflow cannot happen");
    let bufp = unsafe {
        alloc(layout).cast::<u8>()
    };

    for row in list {
        let ext = row.extension();
        // println!("> {:?}", ext_name);
        if None == ext { continue; }

        let filename = row.to_str().unwrap();
        let ext_name = ext.unwrap().to_str().unwrap();
        // println!("\n{:?}", filename);
        match meta::media_type(ext_name) {
            EnMediaType::AUDIO => {
                // println!("{}", 1111);
                let len = meta::load_audio(bufp, filename, 8192);
                println!("{}", len);
                let mut result = unsafe {
                    String::from_raw_parts(bufp, len as usize, 8192)
                };
                println!("{}", result);
                0
            },
            EnMediaType::IMAGE => {
                meta::load_image(filename);
                0
            },
            EnMediaType::UNKNOW => 0
        };
    }

    // unsafe {
    //     free(bufp);
    // }
}
