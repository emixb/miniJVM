
./mini_jvm -cp ../lib/minijvm_rt.jar:../libex/glfw_gui.jar test/AppManagerTest
./mini_jvm -cp ../lib/minijvm_rt.jar:../libex/janino.jar:../libex/commons-compiler.jar org.codehaus.janino.Compiler  ../res/BpDeepTest.java

echo execute BpDeepTest
./mini_jvm -cp ../lib/minijvm_rt.jar:../res/ BpDeepTest

./mini_jvm -cp ../lib/minijvm_rt.jar:../libex/luaj.jar Sample