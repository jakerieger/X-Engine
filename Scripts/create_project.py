import os
import xml.dom.minidom as minidom
import sys
from pathlib import Path

def create_project_file(name: str):
    doc = minidom.getDOMImplementation().createDocument(None, "Project", None)

    project_element = doc.documentElement
    project_element.setAttribute("name", name)

    eng_ver_element = doc.createElement("EngineVersion")
    eng_ver_node = doc.createTextNode("1")
    eng_ver_element.appendChild(eng_ver_node)
    project_element.appendChild(eng_ver_element)

    content_dir_element = doc.createElement("ContentDirectory")
    content_dir_node = doc.createTextNode("Content")
    content_dir_element.appendChild(content_dir_node)
    project_element.appendChild(content_dir_element)

    startup_scene_element = doc.createElement("StartupScene")
    startup_scene_node = doc.createTextNode("Empty")
    startup_scene_element.appendChild(startup_scene_node)
    project_element.appendChild(startup_scene_element)

    with open(f"{name}.xproj", "w") as f:
        f.write(doc.toprettyxml(indent="  "))

    return

def create_content(root: str):
    content_dirs = ["Audio", "Materials", "Models", "Scenes", "Scripts", "Textures"]

    for d in content_dirs:
        os.makedirs(os.path.join(root, d), exist_ok=True)

    return

def create_runtime(name: str):
    return

def create_project():
    projects_root = Path.home() / "Documents" / "XENGINE Projects"
    if not projects_root.exists():
        projects_root.mkdir()

    print(f"Projects directory: {projects_root}")

    project_name = input("Enter a project name: ")

    if not project_name or any(char in r' \/:*?"<>|' for char in project_name):
        print("Invalid project name. Please avoid using special characters.")
        return

    project_dir = projects_root.joinpath(project_name)
    if not project_dir.exists():
        project_dir.mkdir()
    else:
        print("Project already exists. Please choose a different name.")
        return

    os.chdir(project_dir)

    try:
        content_dir = os.path.join(project_dir, "Content")
        runtime_dir = os.path.join(project_dir, "Runtime")
        os.makedirs(content_dir, exist_ok=True)
        os.makedirs(runtime_dir, exist_ok=True)

        create_project_file(project_name)
        create_content(content_dir)
        create_runtime(project_name)

    except Exception as e:
        print(f"Error creating project: {e}")

    return

if __name__ == "__main__":
    create_project()