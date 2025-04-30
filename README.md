hello, welcome to my little kernel thing
no docs but the `help` command in the shell should be of use
in the shell one thing you can do that's not in the help command is `echo something > filename.txt` like on unix/linux systems

how to run on linux:
have both qemu(or specifically qemu-system-i386) and docker installed and working properly
after that you should be able to just run `./buildtest.sh`

how to run on windows:
you'll need git bash(or possibly another bash for windows thing) setup and working, if you have that, follow the readme's in the gcc and qemu folder, they'll help you install portable versions of the tools you'll need to build and test the project, if you've done both of those you *should* be able to run bash and run `./buildtest2.sh`