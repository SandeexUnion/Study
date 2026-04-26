using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Linq;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.IO;

class TorrentClient
{
    static string serverIp;
    static int serverPort = 8888;
    static int p2pPort = 8889;
    static List<byte[]> chunks = new List<byte[]>();
    static bool[] hasChunk;
    static int chunkSize;
    static int totalChunks;
    static string downloadPath = "downloaded_file.pdf";
    static List<string> peers = new List<string>();
    static bool isRunning = true;
    static bool useServer = true;

    static void Main()
    {
        Console.WriteLine("=== P2P Клиент ===");
        Console.Write("IP сервера: ");
        serverIp = Console.ReadLine();
        if (string.IsNullOrEmpty(serverIp)) useServer = false;

        Console.Write("P2P порт: ");
        string p2pInput = Console.ReadLine();
        if (!string.IsNullOrEmpty(p2pInput)) p2pPort = int.Parse(p2pInput);

        if (useServer)
            GetMetaFromServer();
        else
            Console.WriteLine("Режим только P2P");

        StartP2PServer();
        StartPeerDiscovery();
        
        Console.WriteLine("\nПоиск пиров для P2P загрузки...");
        
        int attempts = 0;
        while (isRunning && hasChunk != null && hasChunk.Any(h => !h))
        {
            if (peers.Count > 0)
            {
                Console.WriteLine($"\nПараллельная загрузка с {peers.Count} пиров...");
                DownloadFromPeersParallel();
            }
            else if (useServer && attempts > 2)
            {
                Console.WriteLine("\nПиры не найдены, скачиваю с сервера...");
                DownloadFromServer();
                break;
            }
            
            ShowProgress();
            Thread.Sleep(1000);
            attempts++;
        }

        if (hasChunk != null && hasChunk.All(h => h))
        {
            AssembleFile();
            Console.WriteLine("\nСкачивание завершено!");
            Console.WriteLine("Клиент переходит в режим сида");
            Console.WriteLine("Нажмите Ctrl+C для выхода");
            
            while (true)
            {
                ShowProgress();
                Thread.Sleep(5000);
            }
        }

        Console.WriteLine("Нажмите Enter для выхода");
        Console.ReadLine();
    }

    static void GetMetaFromServer()
    {
        try
        {
            using (var client = new TcpClient(serverIp, serverPort))
            using (NetworkStream ns = client.GetStream())
            {
                StreamWriter writer = new StreamWriter(ns, Encoding.UTF8) { AutoFlush = true };
                writer.WriteLine("META");
                var reader = new StreamReader(ns, Encoding.UTF8);
                string[] meta = reader.ReadLine().Split('|');
                totalChunks = int.Parse(meta[0]);
                chunkSize = int.Parse(meta[1]);

                hasChunk = new bool[totalChunks];
                for (int i = 0; i < totalChunks; i++) chunks.Add(null);

                Console.WriteLine($"Файл: {totalChunks} чанков по {chunkSize / 1024}KB");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Ошибка подключения к серверу: {ex.Message}");
            Environment.Exit(1);
        }
    }

    static void DownloadFromServer()
    {
        if (!useServer) return;
        
        for (int i = 0; i < totalChunks; i++)
        {
            if (hasChunk[i]) continue;

            try
            {
                using (var client = new TcpClient(serverIp, serverPort))
                using (NetworkStream ns = client.GetStream())
                {
                    StreamWriter writer = new StreamWriter(ns, Encoding.UTF8) { AutoFlush = true };
                    writer.WriteLine($"CHUNK:{i}");

                    byte[] lenBuffer = new byte[4];
                    if (ns.Read(lenBuffer, 0, 4) != 4) continue;
                    int len = BitConverter.ToInt32(lenBuffer, 0);

                    byte[] chunkData = new byte[len];
                    int received = 0;
                    while (received < len)
                        received += ns.Read(chunkData, received, len - received);

                    chunks[i] = chunkData;
                    hasChunk[i] = true;
                    Console.WriteLine($"С сервера получен чанк {i + 1}/{totalChunks}");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Ошибка при скачивании чанка {i}: {ex.Message}");
            }
        }
    }

    static void StartP2PServer()
    {
        Task.Run(() =>
        {
            try
            {
                TcpListener listener = new TcpListener(IPAddress.Any, p2pPort);
                listener.Start();
                Console.WriteLine($"P2P сервер запущен на порту {p2pPort}");

                while (true)
                {
                    var client = listener.AcceptTcpClient();
                    Task.Run(() => HandlePeerRequest(client));
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"P2P ошибка: {ex.Message}");
            }
        });
    }

    static void HandlePeerRequest(TcpClient peerClient)
    {
        try
        {
            using (peerClient)
            using (NetworkStream ns = peerClient.GetStream())
            {
                byte[] buffer = new byte[4];
                if (ns.Read(buffer, 0, 4) != 4) return;
                int chunkIndex = BitConverter.ToInt32(buffer, 0);

                if (chunkIndex >= 0 && chunkIndex < totalChunks && hasChunk != null && hasChunk[chunkIndex] && chunks[chunkIndex] != null)
                {
                    byte[] chunk = chunks[chunkIndex];
                    byte[] lenBytes = BitConverter.GetBytes(chunk.Length);
                    ns.Write(lenBytes, 0, 4);
                    ns.Write(chunk, 0, chunk.Length);
                    Console.WriteLine($"Отдал чанк {chunkIndex} пиру");
                }
            }
        }
        catch { }
    }

    static void StartPeerDiscovery()
    {
        Task.Run(() =>
        {
            using (UdpClient udp = new UdpClient())
            {
                udp.EnableBroadcast = true;
                IPEndPoint broadcast = new IPEndPoint(IPAddress.Broadcast, 8887);

                while (isRunning)
                {
                    try
                    {
                        string message = $"HELLO|{p2pPort}";
                        byte[] data = Encoding.UTF8.GetBytes(message);
                        udp.Send(data, data.Length, broadcast);
                    }
                    catch { }
                    Thread.Sleep(3000);
                }
            }
        });

        Task.Run(() =>
        {
            try
            {
                using (UdpClient udpListener = new UdpClient())
                {
                    udpListener.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
                    udpListener.Client.Bind(new IPEndPoint(IPAddress.Any, 8887));
                    
                    while (isRunning)
                    {
                        try
                        {
                            IPEndPoint sender = new IPEndPoint(IPAddress.Any, 0);
                            byte[] data = udpListener.Receive(ref sender);
                            string message = Encoding.UTF8.GetString(data);

                            if (message.StartsWith("HELLO|"))
                            {
                                int peerPort = int.Parse(message.Split('|')[1]);
                                string peerAddress = $"{sender.Address}:{peerPort}";
                                
                                lock (peers)
                                {
                                    if (!peers.Contains(peerAddress) && peerPort != p2pPort)
                                    {
                                        peers.Add(peerAddress);
                                        Console.WriteLine($"\nНайден пир: {peerAddress}");
                                    }
                                }
                            }
                        }
                        catch { }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"UDP ошибка: {ex.Message}");
            }
        });
    }

    static void DownloadFromPeersParallel()
    {
        List<Task> tasks = new List<Task>();
        
        int chunksPerPeer = totalChunks / Math.Max(1, peers.Count);
        int peerIndex = 0;
        
        foreach (string peer in peers.ToList())
        {
            int start = peerIndex * chunksPerPeer;
            int end = (peerIndex == peers.Count - 1) ? totalChunks : start + chunksPerPeer;
            string currentPeer = peer;
            
            tasks.Add(Task.Run(() => DownloadChunksFromPeer(currentPeer, start, end)));
            peerIndex++;
        }
        
        Task.WaitAll(tasks.ToArray());
    }

    static void DownloadChunksFromPeer(string peer, int startChunk, int endChunk)
    {
        try
        {
            string[] parts = peer.Split(':');
            string ip = parts[0];
            int port = int.Parse(parts[1]);

            using (var client = new TcpClient())
            {
                client.Connect(ip, port);
                client.ReceiveTimeout = 2000;
                using (NetworkStream ns = client.GetStream())
                {
                    for (int i = startChunk; i < endChunk; i++)
                    {
                        if (hasChunk[i]) continue;

                        byte[] request = BitConverter.GetBytes(i);
                        ns.Write(request, 0, 4);

                        byte[] lenBuffer = new byte[4];
                        if (ns.Read(lenBuffer, 0, 4) != 4) continue;
                        int len = BitConverter.ToInt32(lenBuffer, 0);

                        byte[] chunkData = new byte[len];
                        int received = 0;
                        while (received < len)
                            received += ns.Read(chunkData, received, len - received);

                        lock (chunks)
                        {
                            if (!hasChunk[i])
                            {
                                chunks[i] = chunkData;
                                hasChunk[i] = true;
                                Console.WriteLine($"\nОт пира {ip}:{port} получен чанк {i + 1}/{totalChunks}");
                            }
                        }
                    }
                }
            }
        }
        catch { }
    }

    static void ShowProgress()
    {
        if (hasChunk == null) return;
        
        int downloaded = hasChunk.Count(h => h);
        int percent = downloaded * 100 / totalChunks;
        string status = hasChunk.All(h => h) ? "СИД" : "ЗАГРУЗКА";
        Console.Write($"\r{status} | {downloaded}/{totalChunks} ({percent}%) | Пиров: {peers.Count}");
    }

    static void AssembleFile()
    {
        try
        {
            using (FileStream fs = new FileStream(downloadPath, FileMode.Create))
                foreach (var chunk in chunks)
                    fs.Write(chunk, 0, chunk.Length);
            Console.WriteLine($"\nФайл сохранён: {downloadPath}");
            Console.WriteLine($"Размер: {new FileInfo(downloadPath).Length} байт");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Ошибка сохранения: {ex.Message}");
        }
    }
}