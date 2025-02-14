import json
import os
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional

@dataclass
class ShaderInfo:
    """Represents information about a shader to be compiled"""
    name: str
    source: str

@dataclass
class ShaderManifest:
    """Represents the complete shader compilation manifest"""
    compiler_path: str
    shader_model: int
    compiled_output: str
    header_output: str
    source_dir: str  # Added source directory field
    graphics_shaders: List[ShaderInfo]
    compute_shaders: List[ShaderInfo]

    @classmethod
    def from_json(cls, json_data: dict) -> 'ShaderManifest':
        """Creates a ShaderManifest instance from parsed JSON data"""
        return cls(
            compiler_path=json_data["compiler"]["fxcPath"],
            shader_model=json_data["shaderModel"],
            compiled_output=json_data["shaders"]["binOutput"],
            header_output=json_data["shaders"]["headerOutput"],
            source_dir=json_data["shaders"]["sourceDir"],  # Parse the source directory
            graphics_shaders=[ShaderInfo(**shader) for shader in json_data["shaders"]["graphics"]],
            compute_shaders=[ShaderInfo(**shader) for shader in json_data["shaders"]["compute"]]
        )

class ShaderCompiler:
    """Handles the compilation of HLSL shaders using FXC"""
    
    def __init__(self, manifest: ShaderManifest):
        self.manifest = manifest
        
    def compile_all(self) -> bool:
        """Compiles all shaders specified in the manifest"""
        # Ensure output directory exists
        os.makedirs(self.manifest.compiled_output, exist_ok=True)
        os.makedirs(self.manifest.header_output, exist_ok=True)
        
        # Convert paths to Path objects for easier manipulation
        source_dir = Path(self.manifest.source_dir)
        
        # Verify source directory exists
        if not source_dir.exists():
            print(f"Error: Source directory does not exist: {source_dir}")
            return False
        
        success = True
        
        # Compile graphics shaders
        for shader in self.manifest.graphics_shaders:
            if not self._compile_graphics_shader(shader, source_dir):
                success = False
                
        # Compile compute shaders
        for shader in self.manifest.compute_shaders:
            if not self._compile_compute_shader(shader, source_dir):
                success = False
                
        return success
    
    def _compile_graphics_shader(self, shader: ShaderInfo, source_dir: Path) -> bool:
        """Compiles a graphics shader (vertex + pixel pair)"""
        print(f"Compiling graphics shader: {shader.name}")
        
        # Resolve full source path
        source_path = source_dir / shader.source
        
        # Verify source file exists
        if not source_path.exists():
            print(f"Error: Shader source file not found: {source_path}")
            return False
        
        # Prepare output paths
        vs_output = Path(self.manifest.compiled_output) / f"{shader.name}_VS.xs"
        ps_output = Path(self.manifest.compiled_output) / f"{shader.name}_PS.xs"
        vs_output_header = Path(self.manifest.header_output) / f"{shader.name}_VS.h"
        ps_output_header = Path(self.manifest.header_output) / f"{shader.name}_PS.h"
        
        # Create output subdirectories if needed
        vs_output.parent.mkdir(parents=True, exist_ok=True)
        ps_output.parent.mkdir(parents=True, exist_ok=True)
        vs_output_header.parent.mkdir(parents=True, exist_ok=True)
        ps_output_header.parent.mkdir(parents=True, exist_ok=True)
        
        # Compile vertex shader
        vs_success = self._run_compiler(
            source=str(source_path),
            name=f"{shader.name}_VS",
            compiled_output=vs_output,
            header_output=vs_output_header,
            profile=f"vs_{self.manifest.shader_model}_0",
            entry_point="VS_Main"
        )
        
        # Compile pixel shader
        ps_success = self._run_compiler(
            source=str(source_path),
            name=f"{shader.name}_PS",
            compiled_output=ps_output,
            header_output=ps_output_header,
            profile=f"ps_{self.manifest.shader_model}_0",
            entry_point="PS_Main"
        )
        
        return vs_success and ps_success
    
    def _compile_compute_shader(self, shader: ShaderInfo, source_dir: Path) -> bool:
        """Compiles a compute shader"""
        print(f"Compiling compute shader: {shader.name}")
        
        # Resolve full source path
        source_path = source_dir / shader.source
        
        # Verify source file exists
        if not source_path.exists():
            print(f"Error: Shader source file not found: {source_path}")
            return False
        
        # Prepare output path
        cs_output = Path(self.manifest.compiled_output) / f"{shader.name}_CS.xs"
        cs_output_header = Path(self.manifest.header_output) / f"{shader.name}_CS.h"
        
        # Create output subdirectories if needed
        cs_output.parent.mkdir(parents=True, exist_ok=True)
        cs_output_header.parent.mkdir(parents=True, exist_ok=True)
        
        # Compile compute shader
        return self._run_compiler(
            source=str(source_path),
            name=f"{shader.name}_CS",
            compiled_output=cs_output,
            header_output=cs_output_header,
            profile=f"cs_{self.manifest.shader_model}_0",
            entry_point="CS_Main"
        )
    
    def _run_compiler(self, source: str, name: str, compiled_output: Path, header_output: Path, profile: str, entry_point: str) -> bool:
        """Runs the FXC compiler with specified parameters"""
        try:
            # TODO: Add the options for debug symbol stripping and optimizations based on build configuration
            cmd = [
                self.manifest.compiler_path,
                "/T", profile,
                "/E", entry_point,
                "/Fo", str(compiled_output),
                "/Fh", str(header_output),
                "/Vn", f"k{name}Bytes",
                source
            ]
            
            # Run FXC compiler
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=False  # Don't raise an exception on non-zero return code
            )
            
            if result.returncode != 0:
                print(f"Error compiling shader: {source}")
                print(f"Command: {' '.join(cmd)}")
                print(f"Error output:\n{result.stderr}")
                return False
                
            return True
            
        except Exception as e:
            print(f"Exception while compiling shader {source}: {str(e)}")
            return False

def resolve_paths(manifest_path: Path, manifest_data: dict) -> dict:
    """
    Resolves all paths in the manifest relative to the manifest file's location.
    This ensures consistent path resolution regardless of working directory.
    """
    # Get the directory containing the manifest
    manifest_dir = manifest_path.parent

    # Create a new copy of the manifest data
    resolved_data = manifest_data.copy()
    
    # Resolve shader directories relative to manifest location
    shaders = resolved_data["shaders"]
    shaders["binOutput"] = str(manifest_dir / shaders["binOutput"])
    shaders["headerOutput"] = str(manifest_dir / shaders["headerOutput"])
    shaders["sourceDir"] = str(manifest_dir / shaders["sourceDir"])
    
    return resolved_data

def main():
    import argparse

    parser = argparse.ArgumentParser(description="Compile HLSL shaders based on a manifest file")
    parser.add_argument("manifest", help="Path to the shader manifest JSON file")
    parser.add_argument(
        "--config",
        choices=['debug', 'release', 'dist'],
        default='debug',
        help="Set configuration mode (default: %(default)s)"
    )

    args = parser.parse_args()
    
    try:
        # Convert manifest path to Path object for easier manipulation
        manifest_path = Path(args.manifest).resolve()
        
        # Read and parse the manifest
        with open(manifest_path, 'r') as f:
            manifest_data = json.load(f)
        
        # Resolve all paths relative to manifest location
        resolved_data = resolve_paths(manifest_path, manifest_data)
            
        # Create manifest object with resolved paths
        manifest = ShaderManifest.from_json(resolved_data)

        # Create compiler and run compilation
        compiler = ShaderCompiler(manifest)
        success = compiler.compile_all()
        
        if success:
            print("All shaders compiled successfully")
            return 0
        else:
            print("Some shaders failed to compile")
            return 1
            
    except Exception as e:
        print(f"Error: {str(e)}")
        return 1

if __name__ == "__main__":
    exit(main())