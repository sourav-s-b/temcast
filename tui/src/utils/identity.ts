import crypto from "node:crypto";
import fs from "node:fs";
import path from "node:path";
import os from "node:os";

const dirPath = path.join(os.homedir(), ".config", "temcast");
const filePath = path.join(dirPath, "device.key");

export function getId() {
  if (!fs.existsSync(filePath)) {
    fs.mkdirSync(dirPath, { recursive: true });

    const { privateKey, publicKey } = crypto.generateKeyPairSync("ed25519");

    const secret = privateKey
      .export({ type: "pkcs8", format: "der" })
      .subarray(-32);
    const pub = publicKey.export({ type: "spki", format: "der" }).subarray(-32);

    const data = Buffer.concat([secret, pub]);

    fs.writeFileSync(filePath, data, { mode: 0o600 });

    const key = crypto.createPrivateKey({
      key: Buffer.concat([
        Buffer.from("302e020100300506032b657004220420", "hex"),
        secret,
      ]),
      format: "der",
      type: "pkcs8",
    });

    const deviceId = pub.toString("hex");

    return { deviceId, key };
  }

  const data = fs.readFileSync(filePath);

  if (data.length != 64) {
    throw new Error("Corrupted device key file")
  }
  const secret = data.subarray(0, 32);

  const key = crypto.createPrivateKey({
    key: Buffer.concat([
      Buffer.from("302e020100300506032b657004220420", "hex"),
      secret,
    ]),
    format: "der",
    type: "pkcs8",
  });

  const pub = data.subarray(32, 64);

  const deviceId = pub.toString("hex");

  return { deviceId, key };
}

export function signing(nonce: string): string {
  const {deviceId , key }= getId();

  const payload = `temcast-auth-v1|${deviceId}|${nonce}`;
  const sig = crypto.sign(null, Buffer.from(payload), key);

  return sig.toString("hex");
}
