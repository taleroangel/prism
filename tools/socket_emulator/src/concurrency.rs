use std::sync::mpsc::{Sender, Receiver, channel};
pub struct BidirectionalChannel<T> {
    pub tx: Sender<T>,
    pub rx: Receiver<T>,
}

impl<T> BidirectionalChannel<T> {
    pub fn new() -> (Self, Self) {
        let (a_tx, b_rx) = channel::<T>();
        let (b_tx, a_rx) = channel::<T>();
        (Self { tx: a_tx, rx: a_rx }, Self { tx: b_tx, rx: b_rx })
    }
}