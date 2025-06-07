;; To use this file to build a version of spoa using git HEAD:
;;
;;   rm -rf target/  # may be necessary when check-for-pregenerated-files fails
;;   guix build -L . -f guix.scm                  # default build
;;
;; To get a development container using a recent guix (see `guix pull`)
;;
;;   guix shell --share=$HOME/.cargo -C -N -L . spoa-shell-git
;;
;; and inside the container
;;
;;   rm -rf target/  # may be necessary when check-for-pregenerated-files fails
;;   CC=gcc cargo build
;;
;; note that we don't expect cargo to download packages, so ignore resolve errors
;;
;; List other packages in this guix.scm file
;;
;;   guix package -L . -A spoa
;;
;; Installing guix (note that Debian comes with guix). Once installed update as a normal user with:
;;
;;   mkdir ~/opt
;;   guix pull -p ~/opt/guix # update guix takes a while - don't do this often!
;;
;; Use the update guix to build spoa:
;;
;;   ~/opt/guix/bin/guix build -L . -f guix.scm
;;
;; Or get a shell (see above)
;;
;; If things do not work you may also have to update the guix-daemon in systemd. Guix mostly downloads binary
;; substitutes. If it wants to build a lot of software you probably have substitutes misconfigured.

;; by Pjotr Prins (c) 2025

(define-module (guix)
  #:use-module ((guix licenses) #:prefix license:)
  ;; #:use-module (guix build-system cmake)
  #:use-module (guix build-system cargo)
  #:use-module (guix download)
  #:use-module (guix gexp)
  #:use-module (guix git-download)
  #:use-module (guix packages)
  #:use-module (guix utils)
  #:use-module (gnu packages algebra)
  #:use-module (gnu packages base)
  #:use-module (gnu packages bash)
  #:use-module (gnu packages bioinformatics)
  #:use-module (gnu packages build-tools)
  #:use-module (gnu packages certs)
  #:use-module (gnu packages cmake)
  #:use-module (gnu packages commencement)
  #:use-module (gnu packages compression)
  #:use-module (gnu packages cpp)
  #:use-module (gnu packages crates-io)
  #:use-module (gnu packages curl)
  #:use-module (gnu packages gcc)
  #:use-module (gnu packages jemalloc)
  #:use-module (gnu packages linux) ; for util-linux column
  #:use-module (gnu packages llvm)
  #:use-module (gnu packages maths)
  #:use-module (gnu packages multiprecision)
  #:use-module (gnu packages perl)
  #:use-module (gnu packages pkg-config)
  #:use-module (gnu packages python)
  #:use-module (gnu packages rust)
  #:use-module (gnu packages rust-apps) ; for cargo
  #:use-module (gnu packages tls)
  #:use-module (gnu packages version-control)
  #:use-module (srfi srfi-1)
  #:use-module (ice-9 popen)
  #:use-module (ice-9 rdelim)
  ;; #:use-module (deps)
  )

(define %source-dir (dirname (current-filename)))

(define %version
  (read-string (open-pipe "git describe --always --tags --long|tr -d $'\n'" OPEN_READ)))


(define-public spoa-base-git
  (package
    (name "spoa-base-git")
    (version %version)
    (source (local-file %source-dir #:recursive? #t))
    (build-system cargo-build-system)
    (inputs (list cmake gcc-toolchain curl gnutls lzip openssl pkg-config zlib xz)) ;; mostly for htslib
    (arguments
     `(#:cargo-inputs (
                       ("rust-cxx" ,rust-cxx-1)
                       ("rust-cxx-build" ,rust-cxx-build-1)
                       )
       ;; #:cargo-development-inputs ()))
       #:cargo-package-flags '("--no-metadata" "--no-verify" "--allow-dirty")
     ))
    (synopsis "spoa-rs")
    (description
     "Rust bindings for spoa")
    (home-page "https://github.com/pangenome/spoa")
    (license license:expat)))

(define-public spoa-shell-git
  "Shell version to use 'cargo build'"
  (package
    (inherit spoa-base-git)
    (name "spoa-shell-git")
    (inputs
     (modify-inputs (package-inputs spoa-base-git)
         (append binutils coreutils-minimal ;; for the shell
                 )))
    (propagated-inputs (list rust rust-cargo nss-certs openssl perl gnu-make-4.2
                             coreutils-minimal which perl binutils gcc-toolchain pkg-config zlib cmake
                             )) ;; to run cargo build in the shell
    (arguments
     `(
       #:cargo-development-inputs ()
       #:cargo-package-flags '("--no-metadata" "--no-verify" "--allow-dirty")
       #:phases (modify-phases %standard-phases
                               (delete 'check-for-pregenerated-files)
                               (delete 'configure)
                               (delete 'build)
                               (delete 'package)
                               (delete 'check)
                               (delete 'install)
                               )))))

spoa-base-git ;; default deployment build with debug info
