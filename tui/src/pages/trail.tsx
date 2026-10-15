import figlet from "figlet";
import { Box, Text, useInput, useStdout } from "ink";
import TextInput from "ink-text-input";
import fs from "node:fs";
import path from "node:path";
import { useState } from "react";

function isValid(input: string): boolean {
  let validMagnet: boolean = false;

  try {
    const url = new URL(input);

    if (url.protocol === "magnet:") {
      const xt = url.searchParams.get("xt");

      if (xt) {
        validMagnet =
          /^urn:btih:[a-zA-Z0-9]{32}$|^urn:btih:[a-fA-F0-9]{40}$/i.test(xt);
      }
    }
  } catch {}

  let validTorrent: boolean = false;

  const filePath = path.resolve(input);
  try {
    validTorrent =
      path.extname(filePath).toLowerCase() === ".torrent" &&
      fs.startSync(filePath).isFile();
  } catch {}

  return validMagnet || validTorrent;
}

export function App() {
  useInput(() => {});
  const { stdout } = useStdout();

  const title = figlet.textSync("Hello World", {
    font: "ANSI Shadow",
  });

  const [valid, setValid] = useState(false);
  const [input, setInput] = useState("");

  const onSubmit = (value: string) => {
    setValid(isValid(value));
  };

  return (
    <Box
      flexDirection="column"
      width={stdout.columns}
      height={stdout.rows}
      alignItems="center"
      justifyContent="center"
    >
      <Text color="magentaBright">{title}</Text>
      <TextInput
        value={input}
        onChange={setInput}
        placeholder="Enter magnet link or torrent path"
        onSubmit={onSubmit}
      />
      <Text>{!valid ? `${input} is invalid` : `${input} is a valid path`}</Text>
    </Box>
  );
}
