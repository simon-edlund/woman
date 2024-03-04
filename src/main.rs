use clap::Parser;

#[derive(Parser, Debug)]
#[clap(author, about, long_about = "Run a woman")]
struct Args {
    woman: String,
}

fn main() {
    let args = Args::parse();

    let woman_path = std::path::PathBuf::from(&args.woman);
    let cmd = std::process::Command::new(&args.woman).spawn();

    println!("{args:?}");
}
