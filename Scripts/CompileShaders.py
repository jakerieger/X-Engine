import json
import os
import subprocess
from pathlib import Path
from typing import Dict, List, Union
import logging

class ShaderCompiler:
    def __init__(self, manifest_path: str, shader_source_dir: str):
        self.manifest_path = manifest_path
        self.shader_source_dir = shader_source_dir
        self.manifest_data = self._load_manifest()
        self.logger = self._setup_logger()

    def _setup_logger(self) -> logging.Logger:
        logger = logging.getLogger("ShaderCompiler")
        logger.setLevel(logging.INFO)
        handler = logging.StreamHandler()
        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        return logger

    def _load_manifest(self) -> Dict:
        try:
            with open(self.manifest_path, 'r') as f:
                manifest = json.load(f)

                required_fields = ["version", "compiler", "shaders"]
                for field in required_fields:
                    if field not in manifest:
                        raise ValueError(f"Missing required field '{field}' in manifest")

                return manifest
        except json.JSONDecodeError as e:
            raise ValueError(f"Invalid JSON in manifest file: {e}")
        except FileNotFoundError:
            raise FileNotFoundError(f"Manifest file not found: {self.manifest_path}")

    def _ensure_output_directory(self, output_dir: str) -> None:
        """Create output directory if it does not exists."""
        Path(output_dir).mkdir(parents=True, exist_ok=True)

    def _compile_shader(self, shader_info: Dict, shader_type: str) -> None:
        """
        Compile an individual shader using FXC

        Args:
            shader_info: Dictionary containing shader information
            shader_type: Type of shader ('graphics' or 'compute')
        """
        fxc_path = self.manifest_data["compiler"]["fxcPath"]
        output_dir = self.manifest_data["shaders"]["output"]
        self._ensure_output_directory(output_dir)

        # TODO: Finish implementing this

def main():
    manifest_path = "Engine/Shaders/manifest.json"
    shader_sources = "Engine/Shaders/Source"

    compiler = ShaderCompiler(manifest_path, shader_sources)
    return 0

if __name__ == "__main__":
    exit(main())