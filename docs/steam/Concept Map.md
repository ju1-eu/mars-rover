# 🗺️ Concept Map: Wissensvernetzung

### Visualisierung (Mermaid Diagramm)

```mermaid
graph TD
    %% Hauptknoten
    Center((GalaxyRVR Profi<br/>Firmware))

    %% ------------------------------
    %% 1. SCIENCE (Naturwissenschaften)
    %% ------------------------------
    subgraph S["1. SCIENCE (Physik & Sensorik)"]
        Physics[Physikalische Prinzipien]
        IMU[IMU Sensor<br/>MPU6050]
        Acc[Accelerometer<br/>Trägheit]
        Gyro[Gyroskop<br/>Drehrate]
        Fusion(Sensor-Fusion<br/>Komplementärfilter)

        Physics --> IMU
        IMU --> Acc
        IMU --> Gyro
        Acc --> Fusion
        Gyro --> Fusion
    end

    %% ------------------------------
    %% 5. MATH (Mathematik)
    %% ------------------------------
    subgraph M["5. MATH (Algorithmen)"]
        Kinematics[Kinematik<br/>Diff. Lenkung]
        Control[Regelungstechnik<br/>P-Regler]
        Integral[Integration<br/>Winkelberechnung]
        Calc_Curve[Kurven-Formel<br/>v_in = v * 1-r]

        Control --> Integral
        Kinematics --> Calc_Curve
    end

    %% ------------------------------
    %% 3. ENGINEERING (Architektur)
    %% ------------------------------
    subgraph E["3. ENGINEERING (Struktur)"]
        Layered[3-Schichten-Architektur]
        HAL[HAL<br/>Hardware Abstraction]
        Logic[Logic Layer<br/>Verhalten]
        App[App Layer<br/>Main FSM]
        PWM_Sol[Lösung:<br/>Hybrid PWM]

        Layered --> HAL
        Layered --> Logic
        Layered --> App
        HAL --> PWM_Sol
    end

    %% ------------------------------
    %% 2. TECHNOLOGY (Informatik)
    %% ------------------------------
    subgraph T["2. TECHNOLOGY (Tools & Time)"]
        CPP[Modern C++<br/>std17 / std20]
        PIO[PlatformIO<br/>Build Env]
        NonBlock[Non-Blocking I/O<br/>millis]
        FSM[Endlicher Automat<br/>States]

        CPP --> PIO
        NonBlock --> FSM
    end

    %% ------------------------------
    %% 4. ARTS (Code Quality)
    %% ------------------------------
    subgraph A["4. ARTS (Gestaltung)"]
        Clean[Clean Code]
        Style[Strong Typing<br/>enum class]
        Doc[Dokumentation<br/>Kanban/Struktur]
        Const[Ressourcen<br/>constexpr]

        Clean --> Style
        Clean --> Const
        Clean --> Doc
    end

    %% VERBINDUNGEN (ZENTRUM)
    Center --- S
    Center --- T
    Center --- E
    Center --- A
    Center --- M

    %% QUERVERBINDUNGEN (LOGIK FLUSS)
    Fusion == "Liefert stabile Winkel" ==> HAL
    HAL -- "Abstrahierte Daten" --> Logic
    Control -- "Korrekturfaktor" --> Logic
    Calc_Curve -- "Motorwerte" --> Logic
    Logic -- "Steuerbefehle" --> App
    FSM -- "Steuert" --> App
    NonBlock -- "Ermöglicht" --> App

    style Center fill:#f9f,stroke:#333,stroke-width:4px
    style Fusion fill:#ff9,stroke:#f66
    style Control fill:#ff9,stroke:#f66
    style FSM fill:#ff9,stroke:#f66
    style Layered fill:#ff9,stroke:#f66
```

-----

### Erläuterung der Zusammenhänge

1.  **Von der Physik zum Code (Science $\rightarrow$ Engineering):**
    Die physikalischen Sensoren (IMU) liefern rohe, verrauschte Daten. Erst die **Sensor-Fusion** (Science) macht daraus nutzbare Daten, die im **HAL** (Engineering) gekapselt werden.

2.  **Von der Mathematik zur Bewegung (Math $\rightarrow$ Logic):**
    Der **Logic Layer** bedient sich mathematischer Konzepte. Der **P-Regler** und die **Kinematik-Formeln** sind die "Gehirnmasse", die entscheidet, wie schnell sich ein Rad drehen muss.

3.  **Vom Tool zur Struktur (Tech $\rightarrow$ App):**
    Ohne das technologische Konzept des **Non-Blocking I/O** (Vermeidung von `delay`) wäre der komplexe **Endliche Automat (FSM)** in der Application-Layer gar nicht möglich. Der Roboter würde sonst während einer Wartezeit "einfrieren".

4.  **Die Form hält alles zusammen (Arts):**
    Damit diese Komplexität handhabbar bleibt, sorgen **Clean Code** und **Dokumentation** dafür, dass Menschen den Code verstehen und warten können.

