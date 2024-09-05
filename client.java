import org.json.JSONObject;

import java.io.*;
import java.net.Socket;

public class TCPClient {

    private static final String SERVER_IP = "127.0.0.1";  // 替换为服务端 IP
    private static final int SERVER_PORT = 8080;

    // 从文件中读取 JSON
    public static String readJsonFromFile(String fileName) {
        StringBuilder jsonContent = new StringBuilder();

        try (BufferedReader br = new BufferedReader(new FileReader(fileName))) {
            String line;
            while ((line = br.readLine()) != null) {
                jsonContent.append(line);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return jsonContent.toString();
    }

    // 处理与服务端的连接并发送数据
    public static void sendJsonToServer(String jsonCommand) {
        try (Socket socket = new Socket(SERVER_IP, SERVER_PORT);
             OutputStream out = socket.getOutputStream();
             InputStream in = socket.getInputStream()) {

            // 发送 JSON 命令
            out.write(jsonCommand.getBytes());
            out.flush();
            System.out.println("JSON command sent: " + jsonCommand);

            // 解析 JSON，检查是否是同步命令
            JSONObject json = new JSONObject(jsonCommand);
            boolean isSync = json.getBoolean("sync");

            // 如果是同步命令，读取服务端返回的数据
            if (isSync) {
                BufferedReader reader = new BufferedReader(new InputStreamReader(in));
                String response;
                StringBuilder serverResponse = new StringBuilder();

                // 持续读取服务端的数据，直到连接关闭
                while ((response = reader.readLine()) != null) {
                    serverResponse.append(response).append("\n");
                }

                System.out.println("Response from server: " + serverResponse.toString());
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        if (args.length != 1) {
            System.err.println("Usage: java TCPClient <json_file>");
            return;
        }

        // 从 JSON 文件中读取内容
        String jsonCommand = readJsonFromFile(args[0]);

        // 将 JSON 发送到服务端
        sendJsonToServer(jsonCommand);
    }
}
