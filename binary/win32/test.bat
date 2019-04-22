rem mini_jvm.exe -cp ../lib/minijvm_rt.jar;../lib/minijvm_test.jar test/Foo1 888
mini_jvm.exe -cp ../lib/minijvm_rt.jar;../libex/glfw_gui.jar test/AppManagerTest

@echo compile BpDeepTest.java
mini_jvm -cp ../lib/minijvm_rt.jar;../libex/janino.jar;../libex/commons-compiler.jar org.codehaus.janino.Compiler  ../res/BpDeepTest.java

@echo execute BpDeepTest
mini_jvm -cp ../lib/minijvm_rt.jar;../res/ BpDeepTest

mini_jvm -cp ../lib/minijvm_rt.jar;../libex/luaj.jar Sample


pause