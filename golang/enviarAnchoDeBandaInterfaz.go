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
        topic := "mike5625/redcasa"

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
                // Obtener la información de ancho de banda utilizando vnstat
                cmd := exec.Command("vnstat", "-tr", "2", "-i", "ue0")
                output, err := cmd.CombinedOutput()
                if err != nil {
                        fmt.Println("Error:", err)
                        return
                }

                // Filtrar los valores de rx y tx
                lines := strings.Split(string(output), "\n")
                var rxValue, txValue string
                for _, line := range lines {
                        if strings.Contains(line, "rx") {
                                rxValue = strings.Fields(line)[1] + " " + strings.Fields(line)[2]
                        } else if strings.Contains(line, "tx") {
                                txValue = strings.Fields(line)[1] + " " + strings.Fields(line)[2]
                        }
                }

                // Formatear los valores y publicar en MQTT
                message := fmt.Sprintf("rx:%s tx:%s", rxValue, txValue)
                token := client.Publish(topic, 0, false, message)
                token.Wait()

                if token.Error() != nil {
                        fmt.Println(token.Error())
                } else {
                        fmt.Println("Bandwidth data published successfully:", message)
                }

                time.Sleep(1 * time.Second) // Esperar antes de volver a medir
        }
}