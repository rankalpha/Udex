package abc.cookie.udex;

public class Config {
    boolean isEnabled;         // 是否开启此配置中的所有设置
    boolean isDumpDex;         // 是否 dump 所有的 dex 文件，每个 dex 保存为一个扩展名为 .dex 的文件
    boolean isDumpCode;        // 是否 dump 所有的 dex 中的类的函数指令代码，每个 dex 保存为一个扩展名为 .idx 的文件（用于修补 dex 文件）
    boolean isDumpCodeByClassList; // 是否只 dump 在 classList 中指定的类的函数指令代码
    String[] classList;        // 在 classList 中指定需要 dump 的类名
    int delaySeconds;          // 是否在启动时延迟相应的秒数，为零表示不延时

    public Config() {
        this.isEnabled = true;
        this.isDumpDex = true;
        this.isDumpCode = false;
        this.isDumpCodeByClassList = false;
        this.classList = new String[]{};
        this.delaySeconds = 0;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(String.format("isEnabled: %b, isDumpDex: %b, isDumpCode: %b, isDumpCodeByClassList: %b, delaySeconds: %d", this.isEnabled, this.isDumpDex, this.isDumpCode, this.isDumpCodeByClassList, this.delaySeconds));
        sb.append(String.format("\nclassList: %d\n", this.classList.length));
        for (int i = 0; i < this.classList.length; i++) {
            sb.append(String.format("  %s\n", this.classList[i]));
        }
        return sb.toString();
    }
}
