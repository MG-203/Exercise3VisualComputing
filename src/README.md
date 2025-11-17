# Assignment 3 – Animated Pickup (Visual Computing PS)

## Team Members
- **Marco Geisler**  
- **Lukas Westram**  
- **Luca Voithofer**

---

## Controls

### **Movement**
| Key | Action |
|-----|--------|
| **W** | Drive forward |
| **S** | Drive backward |
| **A** | Steer left |
| **D** | Steer right |

### **Camera Modes**
| Key | Camera Mode |
|------|-------------|
| **1** | **Orbit Camera** – free rotation and zoom around the scene (default template camera) |
| **2** | **Tower Camera** – fixed position, always looks at the car |
| **3** | **Chase Camera** – dynamic third-person camera behind the car; flips to the front when driving backward |

### **Mouse**
| Input | Action |
|---------|--------|
| **Left Mouse Button + Drag** | Rotate camera |
| **Scroll Wheel** | Zoom in/out |

---

## Additional Information

- Alongside the required camera mode, we implemented an **additional third camera mode** (Key `3`), a dynamic **chase camera** similar to modern racing games. The reason for this is that only after we had implemented our “chase mode” as the second mode did we watch the demo video and realize that the second mode is actually a bit different. We then kept our “chase mode” as the third mode because we had already implemented it. 
- After reviewing the provided demo video, we created the **tower camera** (Key `2`), matching the assignment specification (fixed position, rotating toward the car).
- The project was developed using **WSL (Windows Subsystem for Linux)**.
- Comments and parts of the structure were assisted by **GitHub Copilot**.

