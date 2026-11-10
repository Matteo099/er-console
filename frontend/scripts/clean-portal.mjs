import fs from "fs";
import path from "path";

const dir = "../sd/.core/portal";

if (fs.existsSync(dir)) {
  for (const file of fs.readdirSync(dir)) {
    if (file !== "README.md") {
      fs.rmSync(path.join(dir, file), { recursive: true, force: true });
    }
  }
}