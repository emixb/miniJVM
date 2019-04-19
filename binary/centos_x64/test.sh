
./mini_jvm -cp ../lib/minijvm_rt.jar:../lib/glfw_gui.jar test/Boing
./mini_jvm -cp ../lib/minijvm_rt.jar:../lib/janino.jar:../lib/commons-compiler.jar org.codehaus.janino.Compiler  ../res/BpDeepTest.java

echo execute BpDeepTest
./mini_jvm -cp ../lib/minijvm_rt.jar:../res/ BpDeepTest
./mini_jvm -cp ../lib/minijvm_rt.jar:../lib/luaj.jar Sample
