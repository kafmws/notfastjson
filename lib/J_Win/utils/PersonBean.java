package utils;

public class PersonBean {
    String className = "utils/PersonBean";
    String name;
    int age;
    int stature;
    String job;
    boolean isMarried;

    @Override
    public String toString() {
        return "PersonBean{" +
                "name='" + name + '\'' +
                ", age=" + age +
                ", stature=" + stature +
                ", job='" + job + '\'' +
                ", isMarried=" + isMarried +
                '}';
    }
}
