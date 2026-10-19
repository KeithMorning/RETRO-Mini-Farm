import AVFoundation
import AppKit
import Foundation
let args = CommandLine.arguments
let asset = AVAsset(url: URL(fileURLWithPath: args[1]))
let gen = AVAssetImageGenerator(asset: asset)
gen.appliesPreferredTrackTransform = true
for (idx, s) in args.dropFirst(2).enumerated() {
    let t = CMTime(seconds: Double(s)!, preferredTimescale: 600)
    do {
        let cg = try gen.copyCGImage(at: t, actualTime: nil)
        let rep = NSBitmapImageRep(cgImage: cg)
        let data = rep.representation(using: .png, properties: [:])!
        try data.write(to: URL(fileURLWithPath: "/tmp/gp_\(idx).png"))
        print("/tmp/gp_\(idx).png")
    } catch { print("err \(error)") }
}
