package utils;

public class Bean {
    String className = "utils/Bean";
    String name;
    String url;
    int page;
    AddressBean address;


    public static class AddressBean {
        String className = "utils/Bean$AddressBean";
        String street;
        String city;
        String country;
    }

    @Override
    public String toString() {
        return "Bean{" +
                "className='" + className + '\'' +
                ", name='" + name + '\'' +
                ", url='" + url + '\'' +
                ", page=" + page +
                ", address=" + address +
                '}';
    }
}
