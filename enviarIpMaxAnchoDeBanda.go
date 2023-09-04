package main

import (
        "fmt"
        "os/exec"
        "strings"
        "time"

        MQTT "github.com/eclipse/paho.mqtt.golang"
)

func main() {
        broker := "tcp://broker.emqx.io:1883"
        clientID := "bandwidth-monitor"
        topic := "mike5625/topip"

        opts := MQTT.NewClientOptions()
        opts.AddBroker(broker)
        opts.SetClientID(clientID)

        client := MQTT.NewClient(opts)
        if token := client.Connect(); token.Wait() && token.Error() != nil {
                fmt.Println(token.Error())
                return
        }
        defer client.Disconnect(250)

        for {
                // Obtener la información de ancho de banda utilizando iftop y grep
                cmd := exec.Command("sh", "-c", "timeout 3 iftop -i ue0 | grep -oE '=> ([0-9]{1,3}\\.){3}[0-9]{1,3}'")
                output, err := cmd.CombinedOutput()
                if err != nil {
                        fmt.Println("Error:", err)
                        return
                }

                // Dividir la salida en líneas
                lines := strings.Split(string(output), "\n")

                // Tomar la cuarta línea (índice 3) y eliminar "=>"
                if len(lines) >= 4 {
                        line := strings.TrimSpace(strings.TrimPrefix(lines[3], "=>"))

                        // Formatear los valores y publicar en MQTT
                        message := line
                        token := client.Publish(topic, 0, false, message)
                        token.Wait()

                        if token.Error() != nil {
                                fmt.Println(token.Error())
                        } else {
                                fmt.Println("Ip que utiliza mas ancho de banda publicada:", message)
                        }
                }

                time.Sleep(1 * time.Second) // Esperar antes de volver a medir
        }
}