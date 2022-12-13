extern crate cc;

fn main() {
    cc::Build::new()
        .file("src/sound/base.c")
        .file("src/sound/meta.c")
        .compile("meta.a");
}
