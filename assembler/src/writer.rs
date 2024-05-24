use std::fs::File;
use std::io::Write;
use anyhow::Result;

/// Write binary data to a file
pub fn writebin(path: &str, binary: &Vec<u8>) -> Result<()> {
    // Create a new file to store the result
    let mut file = File::create(path)?;
    // Write the binary data
    file.write_all(binary.as_slice())?;
    Ok(())
}