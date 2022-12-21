extern crate cc;

fn main() {
    cc::Build::new()
        .file("src/meta/pic/jpg.c")
        .file("src/meta/pic/png.c")
        .file("src/meta/pic/pic.c")
        .file("src/meta/pic/main.c")
        .file("src/meta/audio.c")
        .compile("meta.a");

    println!("cargo:rustc-link-search=/usr/lib/x86_64-linux-gnu");
    println!("cargo:rustc-link-lib=avformat");
    println!("cargo:rustc-link-lib=avcodec");
    println!("cargo:rustc-link-lib=avutil");
}
