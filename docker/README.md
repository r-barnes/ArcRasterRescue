# Usage

Docker must be installed. Its recommended that `docker-compose` is also installed for
ease of use. The built container is ~800 mb in size.

1. Create a clone of the repository. Then, change to the directory named `docker/`.

```shell
git clone https://github.com/r-barnes/ArcRasterRescue.git
cd ArcRasterRescue/docker
```

2. Build the docker image. This will download and build the latest release
version of ArcRasterRescue. The image is built using an Ubuntu 18.04 bionic base.

```shell
docker-compose build
```

You can verify that the build was successful using `docker images`. Look for an image
named `arc_raster_rescue`.

3. Once the image is built it's ready to use. Before you use it, its important to
note that the docker container that is spun up from docker image created in the last
step will be mounted locally to the `./docker/` directory. This means the geodatabase
for which you are extracting rasters from must be in the `./docker/` directory. If
you would like to change this skip down to the second example below.

```shell
docker-compose run --rm arc_raster_rescue
root@:/home# arc_raster_rescue.exe <path/to/geodatabase.gdb/> <RASTER NUM> <OUTPUT FILE>
```

If you would like to change the directory that the container is mounted to, you will
need to make a tiny modification to the `docker-compose.yaml` file. The first code
snippet shows the original docker-compose yaml. In this let's say instead the
geodatabase of interest was in the `~/Documents` directory or somewhere beneath it's
hierarchy.

```yaml
# Original docker-compose.yaml
version: "3.8"
services:
  arc_raster_rescue:
    build: .
    image: arc_raster_rescue
    volumes:
      - .:/home/
    entrypoint: "/bin/bash"
```
Notice the small modification to the line after `volumes`. Once this small change has
been made, you can re-compose the container using `docker-compose run --rm
arc_raster_rescue` as shown prior.

```yaml
# Modified docker-compose.yaml
version: "3.8"
services:
  arc_raster_rescue:
    build: .
    image: arc_raster_rescue
    volumes:
      - ~/Downloads:/home/
    entrypoint: "/bin/bash"
```
