mpic++ -o  square square.cc
echo Successfully compiled!
mpiexec -n  5 ./square zar_noisy.txt  output.txt 0.4 0.15
echo Done
python text_to_image.py output.txt output.jpg
