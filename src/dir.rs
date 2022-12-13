use std::{fs, path};

fn visit_dir(root: &path::Path) -> (Vec<path::PathBuf>, Vec<path::PathBuf>) {
    let mut dirs: Vec<path::PathBuf> = Vec::new();
    let mut files: Vec<path::PathBuf> = Vec::new();

    match fs::read_dir(root) {
        Err(why) => println!("! {:?}", why.kind()),
        Ok(paths) => {
            for path in paths {
                let path = path.unwrap().path();
                if path.is_dir() {
                    dirs.push(path);
                } else if path.is_file() {
                    files.push(path);
                }
            }
        }
    }

    (files, dirs)
}

pub fn walk_dir(root: &path::Path, mut dp: i32) -> Vec<path::PathBuf> {
    let (mut files, mut dirs) = visit_dir(root);

    while 0 < dirs.len() && 0 < dp {
        let mut dir_list: Vec<path::PathBuf> = Vec::new();

        for dr in dirs {
            let (mut _files, mut _dirs) = visit_dir(dr.as_path());
            files.extend(_files);
            dir_list.extend(_dirs);
        }

        dirs = dir_list;
        dp -= 1;
    }

    return files;
}
