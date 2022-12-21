mod dir;
mod meta;

use std::{env, path::Path};
use String;
use meta::EnMediaType;

fn main() {
    let mut args = env::args();
    let root = if 1 < args.len() { args.nth(1).unwrap() } else { String::from("/home/james/图片/") };
    // println!("{}", root.as_str());
    let list = dir::walk(Path::new(&root));

    for row in list {
        let ext = row.extension();
        // println!("> {:?}", ext_name);
        if None == ext { continue; }

        let filename = row.to_str().unwrap();
        let ext_name = ext.unwrap().to_str().unwrap();
        println!("\n{:?}", filename);
        match meta::media_type(ext_name) {
            EnMediaType::AUDIO => meta::load_audio(filename),
            EnMediaType::IMAGE => {
                meta::load_image(filename);
                0
            },
            EnMediaType::UNKNOW => 0
        };
    }
}
