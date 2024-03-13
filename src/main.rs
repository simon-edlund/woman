use clap::Parser;
use futures::{
    future::FutureExt, // for `.fuse()`
    pin_mut,
    select,
};
use std::io::BufRead;
use tokio::io::AsyncBufReadExt;

#[derive(Parser, Debug)]
#[clap(author, about, long_about = "Run a woman")]
struct Args {
    woman: String,
}

#[tokio::main]
async fn main() {
    let args = Args::parse();

    let woman_path = std::path::PathBuf::from(&args.woman);
    let mut cmd = tokio::process::Command::new(&args.woman)
        .stdout(std::process::Stdio::piped())
        .stderr(std::process::Stdio::piped())
        .spawn()
        .unwrap();

    //println!("{args:?}");
    //println!("{cmd:?}");

    let stdout = cmd.stdout.take().unwrap();
    let stderr = cmd.stderr.take().unwrap();

    let mut stdout_reader = tokio::io::BufReader::new(stdout);
    let mut outline = String::new();

    let mut stderr_reader = tokio::io::BufReader::new(stderr);
    let mut errline = String::new();

    loop {
        let fo = stdout_reader.read_line(&mut outline).fuse();
        let fe = stderr_reader.read_line(&mut errline).fuse();

        pin_mut!(fo, fe);
        select! {
            r = fo => {
                if r.is_ok() && !outline.is_empty() {
                    let text = outline.trim();
                    println!("o: {text}");
                    outline.clear()
                } else {
                    loop {
                        let r = stderr_reader.read_line(&mut errline).await;
                        if r.is_ok() && !errline.is_empty() {
                            let text = errline.trim();
                            println!("e: {text}");
                            errline.clear()
                        } else {
                            break
                        }
                    }
                    break;
                }
            },
            r = fe => {
                if r.is_ok() && !errline.is_empty() {
                    let text = errline.trim();
                    println!("e: {text}");
                    errline.clear()
                } else {
                    loop {
                        let r = stdout_reader.read_line(&mut outline).await;
                        if r.is_ok() && !outline.is_empty() {
                            let text = outline.trim();
                            println!("o: {text}");
                            outline.clear()
                        } else {
                            break
                        }
                    }
                    break;
                }
            },
        }
    }
}
