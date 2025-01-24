# HSDES (Hardware Software Design of IoT Systems M) A.Y. 2024/25

## Environment setup
The target devices for hands-on explanations are the single-core [PULPissimo](https://github.com/pulp-platform/pulpissimo) and multi-core [PULP](https://github.com/pulp-platform/pulp) platforms. 
The PULP Virtual Platform simulator GVSOC, which is included within the [PULP SDK](https://github.com/pulp-platform/pulp-sdk), will be used during the class. 
The execution environment for this class is distributed as a Docker container available in Docker Hub for both x86_64 (Intel / AMD based machines) and arm64 devices (Apple M-series processors, other devices using Qualcomm SoCs).
The same setup will be used also in the other lab sessions.

### Quick look at first usage
![Alt Text](https://raw.githubusercontent.com/EEESlab/hsdes-2025/refs/heads/main/screencast/setup_screencast.gif)

### Recommended setup: Docker + VSCode + Dev Container
1. Install Docker. You can follow instructions here [Docker Desktop](https://www.docker.com/products/docker-desktop/) and [Docker Engine](https://docs.docker.com/engine/install/) for Linux distros.
2. Install [Visual Studio Code](https://code.visualstudio.com/)
3. Open Visual Studio Code, then through the Extensions tab look for the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension.
4. Open this folder (HSDES-LAB01-PULP_Embedded_Programming) from Visual Studio Code. VSCode will ask you (bottom-right corner) to "Reopen in Container": select yes. The required setup should be automatically downloaded (~1.2GiB).
5. The Visual Studio Code terminal on the bottom will use an already configured workspace. The lab folder will be mounted under `/workspaces`.

### Alternate setup: Docker + Own editor
1. Install Docker. You can follow instructions here [Docker Desktop](https://www.docker.com/products/docker-desktop/) and [Docker Engine](https://docs.docker.com/engine/install/) for Linux distros.
2. Open a terminal. You can pull the Docker container with the following command:
```
docker pull fconti/hsdes-container
```
3. Enter the folder of this lab session, and run the following to enter the container shell.
```
docker run --rm -it -v "$(pwd)":/workspaces -w /workspaces fconti/hsdes-container
```
4. The Docker terminal that is opened will contain an already configured workspace. The lab folder will be mounted under `/workspaces`.
