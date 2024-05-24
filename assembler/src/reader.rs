use std::fs::File;
use std::io::{BufRead, BufReader};
use thiserror::Error;
use anyhow::Result;

#[derive(Debug, Error)]
pub enum PrismReadError {
    #[error("prism file not found or inaccessible")]
    ReadError(#[from] std::io::Error),

    #[error("cannot concatenate instructions")]
    ConcatenationError(),
}

/// Read a new .prism file
pub fn readpf(path: &str) -> Result<String> {
    // Open the file and create the reader
    let file = File::open(path)
        .map_err(move |e| PrismReadError::ReadError(e))?;
    let reader = BufReader::new(file);

    // Read line by line
    let lines: Vec<String> = reader.lines()
        .filter_map(move |x| {
            x.ok()
                // Trim spaces and transform to lowercase ascii
                .map(move |x| x.to_ascii_lowercase().trim().to_string())
                // Delete comments and empty lines
                .filter(move |x| !x.starts_with(";") && !x.is_empty())
        }).collect();

    lines.iter()
        .map(move |t| t.to_string())
        .reduce(move |acc, val| acc + "\n" + &val)
        .ok_or(PrismReadError::ConcatenationError().into())
}