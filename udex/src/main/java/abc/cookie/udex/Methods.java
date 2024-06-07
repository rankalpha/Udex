package abc.cookie.udex;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.ArrayList;

public class Methods {
    public ArrayList<Method[]> methods;
    public ArrayList<Constructor[]> constructors;

    public Methods() {
        this.methods = new ArrayList<Method[]>();
        this.constructors = new ArrayList<Constructor[]>();
    }

    public void AddMethods(Method[] methods) {
        this.methods.add(methods);
    }

    public void AddConstructors(Constructor[] constructors) {
        this.constructors.add(constructors);
    }

    private int getMethodCount() {
        int count = 0;
        for (int i = 0; i < this.methods.size(); i ++) {
            Method[] meth = this.methods.get(i);
            count += meth.length;
        }
        return count;
    }
    public Method[] GetMethods() {
        int count = getMethodCount();
        Method[] allMethods = new Method[count];
        int index = 0;
        for (int i = 0; i < this.methods.size(); i ++) {
            Method[] meth = this.methods.get(i);
            System.arraycopy(meth, 0, allMethods, index, meth.length);
            index += meth.length;
        }
        return allMethods;
    }

    private int getConstructorCount() {
        int count = 0;
        for (int i = 0; i < this.constructors.size(); i ++) {
            Constructor[] ctor = this.constructors.get(i);
            count += ctor.length;
        }
        return count;
    }
    public Constructor[] GetConstructors() {
        int count = getConstructorCount();
        Constructor[] allConstructors = new Constructor[count];
        int index = 0;
        for (int i = 0; i < this.constructors.size(); i ++) {
            Constructor[] ctor = this.constructors.get(i);
            System.arraycopy(ctor, 0, allConstructors, index, ctor.length);
            index += ctor.length;
        }
        return allConstructors;
    }
}
