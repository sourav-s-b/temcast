import { render } from "ink";
import { App } from "./pages/trail.tsx";

const enterAltScreen = () => {
  process.stdout.write("\x1b[?1049h");
};

const exitAltScreen = () => {
  process.stdout.write("\x1b[?1049l");
};

enterAltScreen();

const instance = render(<App />);

const cleanup = () => {
  instance.unmount();
  exitAltScreen();
};

process.on("SIGINT", cleanup);
process.on("SIGTERM", cleanup);
process.on("exit", exitAltScreen);
