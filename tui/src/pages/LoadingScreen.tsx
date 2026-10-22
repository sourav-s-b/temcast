import { Box, Text, useInput, useStdout } from "ink";
import { Spinner } from "@inkjs/ui";
import { useEffect, useState } from "react";
import { getId, signing } from "../utils/identity.js";
import WebSocket from "ws";

type status = "trying" | "success" | "error";

export function LoadingScreen() {
  const { stdout } = useStdout();
  const height = stdout.rows;
  const width = stdout.columns;

  const [res, setRes] = useState<string>("Connecting to server");
  const [status, setStatus] = useState<status>("trying");

  useEffect(() => {
    const ws = new WebSocket("ws://127.0.0.1:6969", {
      headers: {
        "X-TemCast-Protocol": "1",
      },
    });

    ws.on("open", () => {
      setRes("Upgraded to WS, Doing Handshake");
      let deviceId: string;
      try {
        deviceId = getId().deviceId;
      } catch (err) {
        setRes(`Error: ${err}`);
        setStatus("error");
        return; 
      }
      const message = {
        t: "hello",
        v: 1,
        device_id: deviceId,
        hw: "arch",
        name: "sourav",
      };

      ws.send(JSON.stringify(message));
    });

    ws.on("message", (data) => {
      let response;
      try {
        response = JSON.parse(data.toString());
      } catch (_) {
        setRes("Error in parsing message from server as JSON");
        setStatus("error");
        return;
      }
      if (response.t == "challenge") {
        const proof = signing(response.nonce);
        const authMessage = {
          t: "auth",
          v: 1,
          proof: proof,
        };

        ws.send(JSON.stringify(authMessage));
      } else if (response.t == "welcome") {
        setRes("Authentication Completed");
        setStatus("success");
      } else if (response.t == "error") {
        setRes(`Error: ${response.reason}`);
        setStatus("error");
      } else {
        setRes(`unknown message from server:\n${data.toString()}`);
      }
    });

    ws.on("error", (error) => {
      setRes(error.message);
      setStatus("error");
    });

    ws.on("unexpected-response", (_req, _res) => {
      let body = "";
      _res.on("data", (c) => (body += c));
      _res.on("error", (_) => {
        setRes(body);
        setStatus("error");
      });
    });

    ws.on("close", (code, reason) => {
      setStatus("error");
      setRes(`connection closed (code ${code})`);
    });

    return () => ws.close();
  }, []);

  return (
    <Box
      alignItems="center"
      flexDirection="column"
      justifyContent="center"
      width={width}
      height={height}
    >
      {status == "trying" && <Spinner label={res} />}
      {status == "error" && <Text>Couldn't Connect : {res}</Text>}
      {status == "success" && <Text>Connected : {res} </Text>}
    </Box>
  );
}
