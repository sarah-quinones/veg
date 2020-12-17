import subprocess, os

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'
if read_the_docs_build:
    subprocess.call(
        'mkdir -p build && cd build && '
        'cmake -DENABLE_DOCS_ONLY=ON ../.. && '
        'make doxygen-docs',
        shell=True)
    breathe_projects = {'veg': 'build/docs/xml'}

project = "veg"
author = "kitegi"
extensions = ["breathe"]
templates_path = ["_templates"]
exclude_patterns = ["_build"]
html_static_path = []
html_theme = "sphinx_rtd_theme"
breathe_default_project = "veg"
