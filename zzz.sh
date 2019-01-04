mpic++ -o  vertical_only vertical_only.cc
echo Successfully compiled!
mpiexec -n  5 ./vertical_only lena200_noisy.txt  output.txt 0.5 0.2
echo Done
python text_to_image.py output.txt yingyang.jpg
