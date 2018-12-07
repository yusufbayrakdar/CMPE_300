mpic++ -o vertical_only vertical_only.cc
mpiexec -n  3 ./vertical_only yinyang_noisy.txt output.txt 0.4 0.1
python text_to_image.py output.txt yingyang.jpg