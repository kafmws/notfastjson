package utils;

import java.lang.management.ManagementFactory;
import java.lang.management.RuntimeMXBean;

public class JsonUtil {
    static {
        System.loadLibrary("JsonParse");
    }

    public static native boolean notFastParseJson(String json, Object object);

    public static void main(String[] args) {
        final RuntimeMXBean runtime = ManagementFactory.getRuntimeMXBean();
        final String info = runtime.getName();
        final int index = info.indexOf("@");
        if (index != -1) {
            final int pid = Integer.parseInt(info.substring(0, index));
            System.out.println(pid);
        }

        PersonBean bean1 = new PersonBean();
        String json1 = "{\"name\":\"wangzepeng\",\"age\":20,\"stature\":170,\"job\":\"programmer\",\"isMarried\":false}";
        Bean bean2 = new Bean();
        String json2 = "{\"name\":\"BeJson\",\"url\":\"http://www.bejson.com\",\"page\":88,\"address\":{\"street\":\"can_not_tell_you\",\"city\":\"ShanXi\",\"country\":\"China\"}}";

        notFastParseJson(json2, bean2);
        System.out.println(bean2);
    }
}
