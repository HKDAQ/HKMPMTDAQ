### Created by Dr. Benjamin Richards (b.richards@qmul.ac.uk)

### Download base image from repo
FROM tooldaq/tooldaq:base

### Run the following commands as super user (root):
USER root

Run cd ToolDAQApplication; make update; make;

### Open terminal
CMD ["/bin/bash"]