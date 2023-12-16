const TEXT_SIZE: i32 = 20;

use std::sync::{
    atomic::{AtomicBool, Ordering},
    Arc, RwLock,
};

use fltk::{
    app,
    prelude::{DisplayExt, GroupExt, WidgetExt},
    text::{TextBuffer, TextDisplay},
    window::Window,
};

use libnewton::interpreter::Interpreter;

pub fn debugger_thread(interpreter: Arc<RwLock<Interpreter>>, execution_flag: Arc<AtomicBool>) {
    let app = app::App::default().with_scheme(app::Scheme::Gtk);
    app::background(0x00, 0x00, 0x00);

    let mut window = Window::default()
        .with_size(500, 150)
        .with_label("Newton Interpreter Debugger");
	
    let mut display = TextDisplay::default().size_of_parent();
	display.set_text_size(TEXT_SIZE);
    display.set_buffer(TextBuffer::default());

    window.end();
    window.show();

    while app.wait() && execution_flag.load(Ordering::Relaxed) {
        let newton_interpreter = interpreter.read().unwrap();
        display.buffer().unwrap().set_text(
            format!(
                "current_selection: {}\ncurrent_range: {:?}\n{:?}\n{:?}",
                newton_interpreter.current_selection(),
                newton_interpreter.current_range(),
                newton_interpreter.registers(),
                newton_interpreter.variables()
            )
            .as_str(),
        );
    }
}
