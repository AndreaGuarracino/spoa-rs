use cxx_build::CFG;
use std::fs::canonicalize;
use std::path::PathBuf;

fn main() {
    let mut cmake_config = cmake::Config::new("spoa");
    cmake_config
        .define("spoa_install", "OFF")
        .define("spoa_build_exe", "OFF")
        .define("spoa_build_tests", "OFF");

    // Disable -march=native for portable builds
    if std::env::var("PORTABLE").unwrap_or_default() == "1" {
        cmake_config.define("spoa_optimize_for_native", "OFF");
    }

    let out_dir = cmake_config.build_target("spoa").build();

    println!(
        "cargo:rustc-link-search=native={}/build/lib",
        out_dir.display()
    );
    println!("cargo:rustc-link-lib=spoa");

    let spoa_include = canonicalize(PathBuf::from("spoa/include")).unwrap();
    CFG.exported_header_dirs.push(&spoa_include);

    cxx_build::bridge("src/lib.rs")
        .cpp(true)
        .file("cxx/spoa_rs.cpp")
        .flag_if_supported("-std=c++14")
        .compile("spoa_rs");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=cxx/spoa_rs.hpp");
    println!("cargo:rerun-if-changed=cxx/spoa_rs.cpp");
    println!("cargo:rerun-if-changed=spoa/spoa.hpp");
}
