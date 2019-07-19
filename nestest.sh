cmake ./CMakeLists.txt -DNESTEST=ON;
make;
./nes_emulator ./test-sample/cpu/nestest/nestest.nes > ./log/out;
head -n $(wc -l out | sed -e 's/out//g') ./test-sample/cpu/nestest/nestest.log > ./log/in ; 