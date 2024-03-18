use std::{thread, time};

#[allow(clippy::explicit_counter_loop)]
fn main() {
    let mut e: i32 = 0;
    let mut o: i32 = 0;
    for _ in 0..10 {
        println!("out {o}");
        o += 1;

        thread::sleep(time::Duration::from_millis(10));

        eprintln!("err {e}");
        e += 1;
    }
    for _ in 0..10 {
        println!("out {o}");
        o += 1;
    }
}
