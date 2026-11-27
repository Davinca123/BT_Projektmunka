import datetime
import tkinter as tk
from matplotlib.ticker import MaxNLocator
import customtkinter as ctk
import pandas as pd
import serial
from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg,
                                               NavigationToolbar2Tk)
from matplotlib.figure import Figure

ser: serial.Serial | None = None
stats_plot_frame = None
hr_df = pd.DataFrame()
steps_df = pd.DataFrame()
hr_stats = []


def get_hourly_steps_series(steps_df_show: pd.DataFrame) -> pd.Series:
    if steps_df_show.empty:
        return pd.Series(0, index=range(24), dtype=int)

    df = steps_df_show.copy()
    df["Steps"] = pd.to_numeric(df["Steps"], errors="coerce").fillna(0).astype(int)
    df["Hour"] = pd.to_datetime(df["Time"], format="%H:%M:%S", errors="coerce").dt.hour

    prev_steps = df["Steps"].shift(1, fill_value=0)
    df["Delta"] = [
        curr if curr < prev else curr - prev
        for curr, prev in zip(df["Steps"], prev_steps)
    ]

    return df.groupby("Hour")["Delta"].sum().reindex(range(24), fill_value=0).astype(int)


def plot_daily_stats(parent_frame, hr_df_show: pd.DataFrame, hourly_steps: pd.Series, stats_list):
    for widget in parent_frame.winfo_children():
        widget.destroy()

    total_steps = int(hourly_steps.sum())
    stats_text = (
        f"HR stats (min-max-avg): {stats_list if stats_list else 'N/A'} "
        f"| Total daily steps: {total_steps}"
    )
    ctk.CTkLabel(master=parent_frame, text=stats_text, font=("Arial", 12)).pack(pady=5)

    # 1. Pulzus grafikon
    hr_card = ctk.CTkFrame(master=parent_frame)
    hr_card.pack(fill="both", expand=True, pady=5)

    fig_hr = Figure(figsize=(8, 3.2), dpi=100)
    ax_hr = fig_hr.add_subplot(111)
    if not hr_df_show.empty:
        bpm_vals = pd.to_numeric(hr_df_show["BPM"], errors="coerce").dropna()
        times_hr = hr_df_show.loc[bpm_vals.index, "Time"]
        ax_hr.plot(times_hr, bpm_vals, marker="o", color="crimson", label="HR (BPM)")
        ax_hr.tick_params(axis="x", rotation=45, labelsize=8)

        if len(bpm_vals) > 0:
            y_min_hr = float(bpm_vals.min())
            y_max_hr = float(bpm_vals.max())
            margin_hr = max(5.0, (y_max_hr - y_min_hr) * 0.18)
            ax_hr.set_ylim(y_min_hr - margin_hr * 0.5, y_max_hr + margin_hr)

            for x_lbl, y_val in zip(times_hr, bpm_vals):
                ax_hr.annotate(
                    f"{int(y_val)}",
                    xy=(x_lbl, y_val),
                    xytext=(0, 6),
                    textcoords="offset points",
                    ha="center",
                    va="bottom",
                    fontsize=8,
                )

    ax_hr.set_title("Daily Heart Rate")
    ax_hr.set_xlabel("Time")
    ax_hr.set_ylabel("BPM")
    ax_hr.grid(True, linestyle="--", alpha=0.4)
    ax_hr.legend(loc="best")
    fig_hr.tight_layout()

    canvas_hr = FigureCanvasTkAgg(fig_hr, master=hr_card)
    canvas_hr.draw()
    toolbar_hr = NavigationToolbar2Tk(canvas_hr, hr_card)
    toolbar_hr.update()
    toolbar_hr.pack(side="top", fill="x")
    canvas_hr.get_tk_widget().pack(fill="both", expand=True)

    # 2. Lépés hisztogram
    st_card = ctk.CTkFrame(master=parent_frame)
    st_card.pack(fill="both", expand=True, pady=5)

    fig_st = Figure(figsize=(8, 3.2), dpi=100)
    ax_st = fig_st.add_subplot(111)
    hours = list(range(24))
    values = hourly_steps.reindex(hours, fill_value=0).values
    bars = ax_st.bar(
        hours,
        values,
        width=1.0,
        align="edge",
        color="#1f6aa5",
        edgecolor="#14375e",
    )
    ax_st.set_xlim(0, 24)
    ax_st.set_xticks(range(25))
    ax_st.set_xticklabels([f"{h:02d}:00" for h in range(25)], rotation=45, ha="right", fontsize=8)
    ax_st.tick_params(axis="x", pad=2)
    ax_st.yaxis.set_major_locator(MaxNLocator(integer=True))

    max_val = int(values.max()) if len(values) > 0 else 0
    ax_st.set_ylim(0, max(1, int((max_val * 1.18) + 0.999)))

    for bar, val in zip(bars, values):
        if val > 0:
            ax_st.annotate(
                f"{int(val)}",
                xy=(bar.get_x() + bar.get_width() / 2, val),
                xytext=(0, 4),
                textcoords="offset points",
                ha="center",
                va="bottom",
                fontsize=8,
            )

    ax_st.set_title("Daily Steps per Hour")
    ax_st.set_xlabel("Hour")
    ax_st.set_ylabel("Steps")
    ax_st.grid(axis="y", linestyle="--", alpha=0.4)
    fig_st.tight_layout()

    canvas_st = FigureCanvasTkAgg(fig_st, master=st_card)
    canvas_st.draw()
    toolbar_st = NavigationToolbar2Tk(canvas_st, st_card)
    toolbar_st.update()
    toolbar_st.pack(side="top", fill="x")
    canvas_st.get_tk_widget().pack(fill="both", expand=True)


def send_actual_time():
    if not ser or not ser.is_open:
        output_string.set("Turn on communication, before sending time!")
        return
    curr_time = str(datetime.datetime.now())[2:-7]
    message = f"#2 {curr_time}\r\n"
    ser.write(message.encode("utf-8"))


def show_stats():
    global stats_plot_frame, steps_df, hr_stats, hr_df
    receive_all()
    hourly_steps = get_hourly_steps_series(steps_df)

    if stats_plot_frame is None or not stats_plot_frame.winfo_exists():
        stats_plot_frame = ctk.CTkFrame(master=main_frame)
        stats_plot_frame.pack(fill="both", expand=True, padx=10, pady=10)

    plot_daily_stats(stats_plot_frame, hr_df, hourly_steps, hr_stats)


def receive_all():
    global stats_plot_frame, hr_df, steps_df, hr_stats
    if not ser or not ser.is_open:
        output_string.set("Turn on communication, before the requests!")
        return
    ser.write("#1\r\n".encode('utf-8'))
    resp = ser.readline().decode("utf-8", errors="replace").strip()

    if not resp:
        output_string.set("No data")
        return

    splitted = resp.split(";")

    hr_raw = splitted[0].split(",") if len(splitted) > 0 and splitted[0] else []
    hr_bpm = [item.split("-")[0] for item in hr_raw if "-" in item]
    hr_time = [item.split("-")[1] for item in hr_raw if "-" in item]

    step_raw = splitted[1].split(",") if len(splitted) > 1 and splitted[1] else []
    step_num = [item.split("-")[0] for item in step_raw if "-" in item]
    step_time = [item.split("-")[1] for item in step_raw if "-" in item]

    hr_bpm_int = [int(x) for x in hr_bpm if x.strip().isdigit()]
    step_num_int = [int(x) for x in step_num if x.strip().isdigit()]
    hr_stats = [min(hr_bpm_int), max(hr_bpm_int),
                0 if len(hr_bpm_int) == 0 else round(sum(hr_bpm_int) / len(hr_bpm_int), 2)]

    hr_df = pd.DataFrame({"BPM": hr_bpm, "Time": hr_time})
    steps_df = pd.DataFrame({"Steps": step_num_int, "Time": step_time})


def center_window_to_display(screen: ctk, width: int, height: int) -> str:
    screen_width = screen.winfo_screenwidth()
    screen_height = screen.winfo_screenheight()
    x = int((screen_width / 2) - (width / 2))
    y = int((screen_height / 2) - (height / 1.5))
    return f"{width}x{height}+{x}+{y}"


def open_serial_port(port: str = "COM7"):
    global ser
    try:
        ser = serial.Serial(
            port=port,
            baudrate=115200,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1,
        )
        output_string.set("Opened serial port" if ser.is_open else "Failed to open serial port")
    except serial.SerialException as e:
        output_string.set(f"Failed to open serial port: {e}")


def save_csv():
    global hr_df, steps_df, hr_stats, ser
    if not ser:
        output_string.set("Turn on communication, before the requests!")
        return
    receive_all()
    if hr_df.empty or steps_df.empty or hr_stats is None:
        output_string.set("No data received!")
        return
    folder = ctk.filedialog.askdirectory()
    if not folder:
        return

    now = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    hr_df.to_csv(f"{folder}/hr_daily_{now}.csv", index=False)
    steps_df.to_csv(f"{folder}/steps_daily_{now}.csv", index=False)
    hourly_steps = get_hourly_steps_series(steps_df)
    total_steps = int(hourly_steps.sum())
    pd.DataFrame({
        "BPM(min)": hr_stats[0], "BPM(max)": hr_stats[1], "BPM(avg)": hr_stats[2], f"Daily steps({now[:-9]})": total_steps
    }, index=[0]).to_csv(f"{folder}/hr_stats_{now}.csv", index=False)

    output_string.set("Saved daily datas to: " + folder)


def off_communication():
    global ser
    if ser and ser.is_open:
        ser.close()
    status_indicator.configure(fg_color="red")
    status.set("MCU communication is off")


def on_communication():
    port = input_field.get().strip().upper()
    open_serial_port(port if port else "COM7")
    if ser and ser.is_open:
        status_indicator.configure(fg_color="green")
        status.set("MCU communication is running")


if __name__ == "__main__":
    ctk.set_appearance_mode("dark")
    ctk.set_default_color_theme("dark-blue")

    window = ctk.CTk()
    window.title("Smartwatch")
    window.iconbitmap("smartwatch.ico")
    window.minsize(700, 500)
    window.geometry(center_window_to_display(window, 900, 600))
    window.bind("<Escape>", lambda e: window.quit())

    main_frame = ctk.CTkScrollableFrame(
        window,
        corner_radius=10,
        scrollbar_button_color="darkblue",
        scrollbar_button_hover_color="white",
    )
    main_frame.pack(fill="both", expand=True, padx=10, pady=10)

    ctk.CTkLabel(master=main_frame, text="SmartWatch GUI", font=("Arial", 20)).pack()

    status_frame = ctk.CTkFrame(master=main_frame)
    status = tk.StringVar(value="MCU communication is off")
    ctk.CTkLabel(master=status_frame, textvariable=status).pack(side="left", padx=10, pady=10)
    status_indicator = ctk.CTkLabel(master=status_frame, fg_color="red", text="", width=50)
    status_indicator.pack(side="right", padx=10, pady=10)
    status_frame.pack()

    port_frame = ctk.CTkFrame(master=main_frame)
    ctk.CTkLabel(master=port_frame, text="MCU Serial Port").pack(side="left", padx=10, pady=10)
    input_field = ctk.CTkEntry(master=port_frame, width=80)
    input_field.set("COM7")
    input_field.pack(side="right", padx=10, pady=10)
    port_frame.pack()

    btn_frame = ctk.CTkFrame(master=main_frame)
    ctk.CTkButton(master=btn_frame, text="On", command=on_communication).pack(side="left", padx=10, pady=10)
    ctk.CTkButton(master=btn_frame, text="Off", command=off_communication).pack(side="left", padx=10, pady=10)
    ctk.CTkButton(master=btn_frame, text="Send actual time", command=send_actual_time).pack(side="right", padx=10,
                                                                                            pady=10)
    ctk.CTkButton(master=btn_frame, text="Show daily statistics", command=show_stats).pack(side="right", padx=10,
                                                                                            pady=10)
    ctk.CTkButton(master=btn_frame, text="Save datas to CSV", command=save_csv).pack(side="right", padx=10,
                                                                                          pady=10)
    btn_frame.pack(pady=10)

    output_string = tk.StringVar()
    ctk.CTkLabel(main_frame, textvariable=output_string, font=("Arial", 12)).pack(pady=5)

    window.mainloop()

    if ser and ser.is_open:
        ser.close()