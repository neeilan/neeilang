g++ -std=c++11 -o nc nc.cc 
./nc 
echo "nasm 2.09"
./nasm -f macho64 out.asm 
ld -macosx_version_min 10.7.0 -lSystem -o result out.o
./result
echo "nasm 2.13"
nasm -f macho64 out.asm 
ld -macosx_version_min 10.7.0 -lSystem -o result out.o
./result
echo $?
